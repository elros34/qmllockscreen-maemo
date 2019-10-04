#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libintl.h>
#include <locale.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <syslog.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkevents.h>
#include <gdk/gdkx.h>
#include <gconf/gconf-client.h>
#include <pango/pango.h>
#include <dbus/dbus.h>
#include <clockd/libtime.h>
#include <hildon/hildon.h>
#include <hildon/hildon-gtk.h>
#include <sqlite3.h>

#include "systemui.h"
#include "osso-systemui-tklock.h"

#ifdef DEBUG

#define SYSLOG_DEBUG(msg, ...) \
  syslog(LOG_MAKEPRI(LOG_USER, LOG_DEBUG), "%s:%d:" msg "\n", __func__, __LINE__, ##__VA_ARGS__)
#define DEBUG_FN SYSLOG_DEBUG("")

#else
  #define SYSLOG_DEBUG(msg, ...)

#endif

#define DEBUG_FN SYSLOG_DEBUG("")

#define SYSLOG_ERROR(msg, ...) \
  syslog(LOG_MAKEPRI(LOG_USER, LOG_ERR), "%s:%d:" msg "\n", __func__, __LINE__, ##__VA_ARGS__)

#define SYSLOG_WARNING(msg, ...) \
  syslog(LOG_MAKEPRI(LOG_USER, LOG_WARNING), "%s:%d:" msg "\n", __func__, __LINE__, ##__VA_ARGS__)

#define SYSLOG_NOTICE(msg, ...) \
  syslog(LOG_MAKEPRI(LOG_USER, LOG_NOTICE), "%s:%d:" msg "\n", __func__, __LINE__, ##__VA_ARGS__)

static DBusHandlerResult
tklock_dbus_filter(DBusConnection *connection, DBusMessage *message, void *user_data);


typedef struct{
 GtkWidget *window;
 guint grab_timeout_tag;
 tklock_status status;
 GdkWindow *grab_notify;
 int field_10;
 tklock_mode mode;
 int button_event;
 void (*one_input_mode_finished_handler)();
 gulong btn_press_handler_id;
 gulong btn_release_handler_id;
 DBusConnection *systemui_conn;
 int field_2C;
 gboolean window_hidden;
} tklock;

typedef struct {
  GtkWidget *time_label;
  GtkWidget *date_label;
} vtklockts;

typedef struct {
  guint count;
  guint hint;
}event_t;

typedef struct {
  GtkWidget *window;
  vtklockts ts;
  GtkWidget *slider;
  guint slider_status;
  int field_14;
  gdouble slider_value;
  GtkAdjustment *slider_adjustment;
  DBusConnection *systemui_conn;
  int priority;
  guint update_date_time_cb_tag;
  void(*unlock_handler)();
  int field_34;
  event_t event[6];
}vtklock_t;

typedef struct{
  system_ui_data *data;
  guint tklock_timeout_tag;
  Window hamm_window;
  gboolean display_off;
  guint cb_argc;
  tklock *gp_tklock;
  vtklock_t *vtklock;
  int field_1C;
} tklock_plugin_data;

tklock_plugin_data *plugin_data = NULL;
system_ui_callback_t system_ui_callback;

guint grab_tries_cnt = 0;

guint event_count = 0;
guint g_notifications[6] = {0,};
time_t g_notifications_mtime;

static void
visual_tklock_destroy_lock(vtklock_t *vtklock);
static void
visual_tklock_create_view_whimsy(vtklock_t *vtklock);
static vtklock_t*
visual_tklock_new(tklock *gp_tklock);
static void
visual_tklock_present_view(vtklock_t *vtklock);
static void
vtklock_unlock_handler();
static void
visual_tklock_set_unlock_handler(vtklock_t *vtklock, void(*unlock_handler)());
static gboolean
vtklock_key_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data);

static void
tklock_destroy_hamm_window();
static void
tklock_create_hamm_window();
static void
gp_tklock_destroy_lock(tklock *gp_tklock);
static gboolean
tklock_timeout_cb(gpointer user_data);
static void
remove_tklock_timeout();

static void
tklock_unlock_display(DBusConnection *conn)
{
  DBusMessage *message;
  const char * unlock = MCE_TK_UNLOCKED;

  DEBUG_FN;
  message = dbus_message_new_method_call(
          MCE_SERVICE,
          MCE_REQUEST_PATH,
          MCE_REQUEST_IF,
          MCE_DISPLAY_ON_REQ);

  if(message)
  {
    dbus_message_set_no_reply(message, TRUE);
    dbus_connection_send(conn, message, 0);
    dbus_connection_flush(conn);
    dbus_message_unref(message);
  }

  message = dbus_message_new_method_call(
         MCE_SERVICE,
         MCE_REQUEST_PATH,
         MCE_REQUEST_IF,
         MCE_TKLOCK_MODE_CHANGE_REQ);

  if(message)
  {
    dbus_message_append_args(message,
                             DBUS_TYPE_STRING, &unlock,
                             DBUS_TYPE_INVALID);

    dbus_message_set_no_reply(message, TRUE);
    dbus_connection_send(conn, message, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(message);
  }
}

/* FIXME who resets grab_tries_cnt on success path? */
gboolean
tklock_grab_timeout_cb(gpointer user_data)
{
  tklock *gp_tklock;

  gp_tklock = (tklock *)user_data;
  DEBUG_FN;
  g_assert(gp_tklock != NULL);

  gdk_pointer_ungrab(0);
  gdk_keyboard_ungrab(0);

  if(grab_tries_cnt == 0)
  {
    gtk_window_close_other_temporaries(GTK_WINDOW(gp_tklock->window));
  }

  if((GDK_GRAB_SUCCESS != gdk_pointer_grab(gp_tklock->window->window,
                                           FALSE, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
                                           0, NULL, 0)) ||
     (GDK_GRAB_SUCCESS != gdk_keyboard_grab(gp_tklock->window->window, TRUE, 0))
     )
  {
    grab_tries_cnt++;

    if(grab_tries_cnt > 3)
    {
      SYSLOG_ERROR("GRAB FAILED (systemui grab), gp_tklock can't be enabled");
      tklock_unlock_display(gp_tklock->systemui_conn);
      gp_tklock->mode = TKLOCK_MODE_NONE;
      gp_tklock->status = TKLOCK_STATUS_RETRY;
      gp_tklock->grab_timeout_tag = 0;
      grab_tries_cnt = 0;
      return FALSE;
    }

    return TRUE;
  }

  gtk_grab_add(gp_tklock->window);

  gp_tklock->status = TKLOCK_STATUS_UNLOCK;
  gp_tklock->grab_timeout_tag = 0;

  return FALSE;
}

static void
tklock_ungrab(tklock *gp_tklock)
{
  DEBUG_FN;

  g_assert(gp_tklock != NULL);

  if(gp_tklock->grab_timeout_tag)
  {
    g_source_remove(gp_tklock->grab_timeout_tag);
    gp_tklock->grab_timeout_tag = 0;
  }

  gdk_pointer_ungrab(0);
  gdk_keyboard_ungrab(0);
  gtk_grab_remove(gp_tklock->window);
}

static void
gp_tklock_enable_lock(tklock *gp_tklock)
{
  DEBUG_FN;

  g_assert(gp_tklock != NULL);

  if(gp_tklock->window_hidden)
  {
    gtk_widget_show(gp_tklock->window);
    gtk_window_move(GTK_WINDOW(gp_tklock->window), -15, -15);
    gp_tklock->window_hidden = FALSE;
  }
}

static void
gp_tklock_disable_lock(tklock *gp_tklock)
{
  DEBUG_FN;

  g_assert(gp_tklock != NULL);

  if(gp_tklock->field_10)
  {
    g_source_remove(gp_tklock->field_10);
    gp_tklock->field_10 = 0;
  }

  if(gp_tklock->grab_notify)
  {
    gdk_window_destroy(gp_tklock->grab_notify);
    gp_tklock->grab_notify = NULL;
  }

  if(gp_tklock->grab_timeout_tag)
  {
    g_source_remove(gp_tklock->grab_timeout_tag);
    gp_tklock->grab_timeout_tag = 0;
  }

  if(gp_tklock->status == TKLOCK_STATUS_UNLOCK)
  {
    tklock_ungrab(gp_tklock);
    gp_tklock->status = TKLOCK_STATUS_NONE;
  }

  if(!gp_tklock->window_hidden)
  {
    gtk_widget_hide(gp_tklock->window);
    gp_tklock->window_hidden = TRUE;
  }

  gdk_error_trap_push();
  gdk_window_invalidate_rect(gdk_get_default_root_window(), NULL, TRUE);
  gdk_flush();
  gdk_error_trap_pop();
}

static void
tklock_map_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  tklock *gp_tklock;

  DEBUG_FN;

  gp_tklock = (tklock *)user_data;

  g_assert(gp_tklock != NULL);

  if(gdk_pointer_is_grabbed())
  {
    SYSLOG_ERROR("GRAB FAILED (systemui grab), gp_tklock can't be enabled");
    tklock_unlock_display(gp_tklock->systemui_conn);
    gp_tklock->mode = TKLOCK_MODE_NONE;
    gp_tklock->status = TKLOCK_STATUS_RETRY;
    return;
  }


  if((GDK_GRAB_SUCCESS != gdk_pointer_grab(widget->window,
                                           FALSE, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
                                           gp_tklock->window->window, NULL, 0)) ||
     (GDK_GRAB_SUCCESS != gdk_keyboard_grab(widget->window, TRUE, 0))
     )
  {
    if(!gp_tklock->grab_timeout_tag)
    {
      gp_tklock->grab_timeout_tag = g_timeout_add(200, tklock_grab_timeout_cb, gp_tklock);
      return;
    }
  }

  gp_tklock->status = TKLOCK_STATUS_UNLOCK;
  gtk_grab_add(widget);
}

gboolean tklock_one_input_mode_finished_handler(tklock *gp_tklock)
{
  DEBUG_FN;

  g_assert(gp_tklock != NULL);
  g_assert(gp_tklock->window != NULL);

  if(!gp_tklock->window_hidden)
  {
    gtk_widget_hide(gp_tklock->window);
    gp_tklock->window_hidden = TRUE;
  }

  if(!gp_tklock->one_input_mode_finished_handler)
  {
    SYSLOG_ERROR("one_input_mode_finished_handler wasn't registered, nop");
  }
  else
    gp_tklock->one_input_mode_finished_handler();

  tklock_ungrab(gp_tklock);

  return FALSE;
}

static gboolean
tklock_key_press_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  tklock *gp_tklock;
  DBusMessage *message;

  DEBUG_FN;

  gp_tklock = (tklock *)user_data;

  g_assert(gp_tklock != NULL);
  g_assert(gp_tklock->systemui_conn != NULL);

  if(gp_tklock->mode != TKLOCK_MODE_NONE)
  {
    tklock_one_input_mode_finished_handler(gp_tklock);
    return TRUE;
  }

  if(event->key.type != GDK_KEY_PRESS)
    return TRUE;

  if(event->key.keyval == GDK_Execute)
    return TRUE;

  if(
     event->key.hardware_keycode ==  73 ||
     event->key.hardware_keycode ==  74 ||
     event->key.hardware_keycode == 121 ||
     event->key.hardware_keycode == 122 ||
     event->key.hardware_keycode == 123 ||
     event->key.hardware_keycode == 171 ||
     event->key.hardware_keycode == 172 ||
     event->key.hardware_keycode == 173 ||
     event->key.hardware_keycode == 208 ||
     event->key.hardware_keycode == 209
     )
  {
    message = dbus_message_new_signal(TKLOCK_SIGNAL_PATH,
                                      TKLOCK_SIGNAL_IF,
                                      TKLOCK_MM_KEY_PRESS_SIG
                                      );

    if(dbus_message_append_args(message,
                                DBUS_TYPE_UINT32, event->key.hardware_keycode,
                                DBUS_TYPE_UINT32, event->key.keyval,
                                DBUS_TYPE_INVALID))
    {
      dbus_connection_send(gp_tklock->systemui_conn, message, 0);
    }

    dbus_message_unref(message);
  }

  return TRUE;
}

static gboolean
tklock_button_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  tklock *gp_tklock;

  DEBUG_FN;

  gp_tklock = (tklock *)user_data;

  g_assert(gp_tklock != NULL);

  if(gp_tklock->mode == TKLOCK_MODE_NONE)
    return TRUE;

  if(event->button.type == GDK_BUTTON_PRESS)
  {
    /* FIXME */
    gp_tklock->button_event = 1;
  }
  else if(event->button.type == GDK_BUTTON_RELEASE)
  {
    tklock_one_input_mode_finished_handler(gp_tklock);
    /* FIXME */
    gp_tklock->button_event = 2;
  }

  return TRUE;
}


static void
gp_tklock_create_window(tklock *gp_tklock)
{
  GdkGeometry geometry = {
    .min_width = 15,
    .min_height = 15,
    .max_width = 15,
    .max_height = 15
  };

  DEBUG_FN;

  g_assert(gp_tklock != NULL);

  gp_tklock->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(gp_tklock->window), "gp_tklock");
  gtk_window_set_decorated(GTK_WINDOW(gp_tklock->window), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW(gp_tklock->window), TRUE);
  gtk_window_set_geometry_hints(GTK_WINDOW(gp_tklock->window),
                                gp_tklock->window,
                                &geometry,
                                GDK_HINT_MAX_SIZE | GDK_HINT_MIN_SIZE);

  g_signal_connect_data(G_OBJECT(gp_tklock->window),
                        "map-event",
                        G_CALLBACK(tklock_map_cb),
                        gp_tklock,
                        NULL,
                        G_CONNECT_AFTER);
  gtk_widget_realize(gp_tklock->window);
  gdk_window_set_events(gp_tklock->window->window,
                        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  g_signal_connect_data(G_OBJECT(gp_tklock->window),
                        "key-press-event",
                        G_CALLBACK(tklock_key_press_cb),
                        gp_tklock,
                        NULL,
                        0);
  gp_tklock->btn_press_handler_id = g_signal_connect_data(G_OBJECT(gp_tklock->window),
                                                          "button-press-event",
                                                          G_CALLBACK(tklock_button_cb),
                                                          gp_tklock,
                                                          NULL,
                                                          0);
  gp_tklock->btn_release_handler_id = g_signal_connect_data(G_OBJECT(gp_tklock->window),
                                                            "button-release-event",
                                                            G_CALLBACK(tklock_button_cb),
                                                            gp_tklock,
                                                            NULL,
                                                            0);
  gdk_window_set_override_redirect(gp_tklock->window->window, TRUE);

}

static tklock *
gp_tklock_init(DBusConnection *systemui_conn)
{
  tklock *gp_tklock;

  DEBUG_FN;

  gp_tklock = g_slice_alloc0(sizeof(tklock));

  if(!gp_tklock)
  {
    SYSLOG_ERROR("failed to allocate memory");
    return NULL;
  }

  gp_tklock->systemui_conn = systemui_conn;

  g_assert(gp_tklock->systemui_conn);

  gp_tklock_create_window(gp_tklock);

  gp_tklock->status = TKLOCK_STATUS_NONE;
  gp_tklock->grab_notify = FALSE;
  gp_tklock->grab_timeout_tag = 0;
  gp_tklock->window_hidden = TRUE;

  return gp_tklock;
}

static void gp_tklock_set_one_input_mode_handler(tklock *gp_tklock, void (*handler)())
{
  g_assert( gp_tklock != NULL);

  DEBUG_FN;

  gp_tklock->one_input_mode_finished_handler = handler;
}

static void
one_input_mode_handler()
{
  DEBUG_FN;

  systemui_do_callback(plugin_data->data, &system_ui_callback, 1);
  systemui_do_callback(plugin_data->data, &system_ui_callback, 4);
  systemui_free_callback(&system_ui_callback);
}

static int
tklock_open_handler(const char *interface,
                    const char *method,
                    GArray *args,
                    system_ui_data *data,
                    system_ui_handler_arg *out)
{
  int supported_args[3] = {'u', 'b', 'b'};
  system_ui_handler_arg* hargs = ((system_ui_handler_arg*)args->data);
  int rv;

  DEBUG_FN;

  if( !check_plugin_arguments(args, supported_args, 1) &&
      !check_plugin_arguments(args, supported_args, 2) &&
      !check_plugin_arguments(args, supported_args, 3) )
  {
    return 0;
  }

  SYSLOG_DEBUG("hargs[4].data.u32[%u]", hargs[4].data.u32);

  switch(hargs[4].data.u32)
  {
    case 4:
    {
      if(plugin_data->gp_tklock == NULL)
        plugin_data->gp_tklock = gp_tklock_init(data->session_bus);

      if(plugin_data->gp_tklock->window == NULL)
        gp_tklock_create_window(plugin_data->gp_tklock);

      if(!plugin_data->gp_tklock->one_input_mode_finished_handler)
        gp_tklock_set_one_input_mode_handler(plugin_data->gp_tklock,one_input_mode_handler);

      plugin_data->cb_argc = 4;
      plugin_data->gp_tklock->mode = TKLOCK_MODE_ENABLE;
      plugin_data->gp_tklock->button_event = 0;
      gp_tklock_enable_lock(plugin_data->gp_tklock);
      break;
    }

    case 5:
    {
      tklock_destroy_hamm_window();

      if(!plugin_data->vtklock)
      {
        plugin_data->vtklock = visual_tklock_new(plugin_data->gp_tklock);
        visual_tklock_set_unlock_handler(plugin_data->vtklock, vtklock_unlock_handler);
      }

      if(!plugin_data->vtklock->window)
        visual_tklock_create_view_whimsy(plugin_data->vtklock);

      visual_tklock_present_view(plugin_data->vtklock);

      if(plugin_data->cb_argc == 1)
      {
        if(plugin_data->gp_tklock->grab_timeout_tag)
        {
          g_source_remove(plugin_data->gp_tklock->grab_timeout_tag);
          plugin_data->gp_tklock->grab_timeout_tag = 0;
        }
        gp_tklock_disable_lock(plugin_data->gp_tklock);
      }

      plugin_data->cb_argc = 5;
      break;
    }
  case 1:
    {
      if(plugin_data->cb_argc == 4)
      {
        do_callback(plugin_data->data, &system_ui_callback, plugin_data->cb_argc);
      }
      else if(plugin_data->cb_argc == 5)
      {
        if(plugin_data->vtklock && plugin_data->vtklock->window)
          visual_tklock_destroy_lock(plugin_data->vtklock);
      }

      if(!plugin_data->hamm_window)
        tklock_create_hamm_window();

      if(!plugin_data->gp_tklock)
        plugin_data->gp_tklock = gp_tklock_init(data->session_bus);
      else if(!plugin_data->gp_tklock->window)
        gp_tklock_create_window(plugin_data->gp_tklock);

      plugin_data->gp_tklock->mode = TKLOCK_MODE_NONE;
      gp_tklock_enable_lock(plugin_data->gp_tklock);
      plugin_data->cb_argc = 1;

      remove_tklock_timeout();

      plugin_data->tklock_timeout_tag = g_timeout_add_seconds(2, tklock_timeout_cb, NULL);
      break;
    }
  default:
    return 0;
  }

  out->arg_type = 'i';

  rv = check_set_callback(args, &system_ui_callback);

  if(rv)
    out->data.i32 = -3;
  else
    out->data.i32 = -2;

  return 'i';
}

static int
tklock_close_handler(const char *interface,
                     const char *method,
                     GArray *args,
                     system_ui_data *data,
                     system_ui_handler_arg *out)
{
  int supported_args[3] = {'b'};
  int mode;
  system_ui_handler_arg* hargs = ((system_ui_handler_arg*)args->data);

  DEBUG_FN;

  if(check_plugin_arguments(args, supported_args, 1))
    mode = hargs[4].data.u32;
  else
    mode = 1;

  tklock_destroy_hamm_window();
  remove_tklock_timeout();

  if(!plugin_data->data)
  {
    SYSLOG_WARNING("tklock wasn't initialized, nop");
    goto out;
  }

  if(plugin_data->gp_tklock)
  {
    if(plugin_data->gp_tklock->mode != TKLOCK_MODE_NONE)
    {
      /* FIXME - WTF, why that check and nothing :( */
      g_signal_handler_is_connected(plugin_data->gp_tklock->window,
                                    plugin_data->gp_tklock->btn_press_handler_id);
    }

    if(plugin_data->gp_tklock->button_event !=2 && mode)
      goto out;

    plugin_data->gp_tklock->mode = TKLOCK_MODE_NONE;
    plugin_data->gp_tklock->button_event = 0;

    if(!plugin_data->gp_tklock->window_hidden)
      gp_tklock_destroy_lock(plugin_data->gp_tklock);

  }

  if(plugin_data->vtklock)
    visual_tklock_destroy_lock(plugin_data->vtklock);

  systemui_free_callback(&system_ui_callback);
out:
  return 'v';
}


static void
tklock_create_hamm_window()
{
  Display *dpy;
  GdkScreen *screen;
  XVisualInfo vinfo;
  Colormap cmap;
  XSetWindowAttributes attributes;
  Atom atom_fullscreen;

  const guint layer = 10;

  DEBUG_FN;

  dpy = gdk_x11_display_get_xdisplay(gdk_display_get_default());
  screen = gdk_screen_get_default();

  XMatchVisualInfo(dpy, DefaultScreen(dpy), 32, VisualDepthMask, &vinfo);

  cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), vinfo.visual, AllocNone);

  attributes.colormap = cmap;
  attributes.override_redirect = 1;
  /* FIXME XBlackPixel?*/
  attributes.border_pixel = 0;
  attributes.background_pixel = 0;

  plugin_data->hamm_window =  XCreateWindow(dpy,
                                            DefaultRootWindow(dpy),
                                            0, 0,
                                            gdk_screen_get_width(screen), gdk_screen_get_height(screen),
                                            0,
                                            32,
                                            InputOutput,
                                            vinfo.visual,
                                            CWBackPixel|CWBorderPixel|CWOverrideRedirect|CWColormap,
                                            &attributes);
  if(!plugin_data->hamm_window)
  {
    SYSLOG_WARNING("Couldn't create Hamm window -> destroy colormap");
    if(cmap)
      XFreeColormap(dpy, cmap);
    return;
  }

  atom_fullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

  XChangeProperty(dpy,
                  plugin_data->hamm_window,
                  XInternAtom(dpy, "_NET_WM_STATE", False),
                  XA_ATOM,
                  32,
                  PropModeReplace,
                  (unsigned char*)&atom_fullscreen,
                  1);

  XChangeProperty(dpy,
                  plugin_data->hamm_window,
                  XInternAtom(dpy, "_HILDON_STACKING_LAYER", False),
                  XA_CARDINAL,
                  32,
                  PropModeReplace,
                  (unsigned char*)&layer,
                  1);

  XMapWindow(dpy, plugin_data->hamm_window);
  XFreeColormap(dpy, cmap);
}

static void
tklock_destroy_hamm_window()
{
  DEBUG_FN;

  if(plugin_data && plugin_data->hamm_window)
  {
    XUnmapWindow(gdk_x11_display_get_xdisplay (gdk_display_get_default()), plugin_data->hamm_window);
    XDestroyWindow(gdk_x11_display_get_xdisplay (gdk_display_get_default()), plugin_data->hamm_window);

    plugin_data->hamm_window = 0;
  }
}

static gboolean
tklock_setup_plugin(system_ui_data *data)
{
  DEBUG_FN;

  plugin_data = g_slice_alloc0(sizeof(tklock_plugin_data));
  if(!plugin_data)
  {
    SYSLOG_ERROR("failed to allocate memory for the plugin data");
    return FALSE;
  }
  plugin_data->data = data;

  plugin_data->field_1C = 0;
  plugin_data->tklock_timeout_tag = 0;
  plugin_data->gp_tklock = NULL;
  plugin_data->vtklock = NULL;

  return TRUE;
}

gboolean
plugin_init(system_ui_data *data)
{
  openlog("systemui-tklock", LOG_ALERT | LOG_USER, LOG_NDELAY);

  DEBUG_FN;

  if( !data )
  {
    SYSLOG_ERROR("initialization parameter value is invalid");
    return FALSE;
  }

  g_return_val_if_fail(tklock_setup_plugin(data), FALSE);

  systemui_add_handler(SYSTEMUI_TKLOCK_OPEN_REQ,
                       tklock_open_handler,
                       data);

  systemui_add_handler(SYSTEMUI_TKLOCK_CLOSE_REQ,
                       tklock_close_handler,
                       data);


  dbus_connection_add_filter(data->session_bus,
                             tklock_dbus_filter,
                             NULL,
                             NULL);

  dbus_bus_add_match(data->session_bus,
                     "type='signal',path='/com/nokia/mce/signal',interface='com.nokia.mce.signal',member='display_off'",
                     NULL);

  return TRUE;
}
static void
gp_tklock_destroy_lock(tklock *gp_tklock)
{
  DEBUG_FN;

  if(gp_tklock->field_10)
  {
    g_source_remove(gp_tklock->field_10);
    gp_tklock->field_10 = 0;
  }

  if(gp_tklock->grab_notify)
  {
    /* FIXME what is the type of gp_tklock->grab_notify?*/
    gdk_window_destroy(gp_tklock->grab_notify);
    gp_tklock->grab_notify = 0;
  }

  if(gp_tklock->grab_timeout_tag)
  {
    g_source_remove(gp_tklock->grab_timeout_tag);
    gp_tklock->grab_timeout_tag = 0;
  }

  if(gp_tklock->status == TKLOCK_STATUS_UNLOCK)
  {
    tklock_ungrab(gp_tklock);
    gp_tklock->status = TKLOCK_STATUS_NONE;
  }

  if(!gp_tklock->window_hidden)
  {
    gtk_widget_hide(gp_tklock->window);
    gp_tklock->window_hidden = TRUE;
  }

  gdk_error_trap_push();
  gdk_window_invalidate_rect(gdk_get_default_root_window(), NULL, TRUE);
  gdk_flush();
  gdk_error_trap_pop();
  gtk_widget_unrealize(gp_tklock->window);
  gtk_widget_destroy(gp_tklock->window);


}


static void
gp_tklock_destroy(tklock *gp_tklock)
{
  DEBUG_FN;

  if(!gp_tklock)
    return;

  if(gp_tklock->field_10)
  {
    g_source_remove(gp_tklock->field_10);
    gp_tklock->field_10 = 0;
  }

  if(gp_tklock->grab_notify)
  {
    /* FIXME what is the type of gp_tklock->grab_notify?*/
    gdk_window_destroy(gp_tklock->grab_notify);
    gp_tklock->grab_notify = 0;
  }

  if(gp_tklock->grab_timeout_tag)
  {
    g_source_remove(gp_tklock->grab_timeout_tag);
    gp_tklock->grab_timeout_tag = 0;
  }

  if(gp_tklock->status == TKLOCK_STATUS_UNLOCK)
  {
    tklock_ungrab(gp_tklock);
    gp_tklock->status = TKLOCK_STATUS_NONE;
  }

  if(!gp_tklock->window_hidden)
  {
    gtk_widget_hide(gp_tklock->window);
    gp_tklock->window_hidden = TRUE;
  }

  gdk_error_trap_push();
  gdk_window_invalidate_rect(gdk_get_default_root_window(), NULL, TRUE);
  gdk_flush();
  gdk_error_trap_pop();
  gtk_widget_unrealize(gp_tklock->window);
  gtk_widget_destroy(gp_tklock->window);

  /* XXX FIXME this comes from the original code, but is useless, as we have that zeroed just a few lines above */
  g_assert(gp_tklock->grab_notify == 0);

  g_slice_free(tklock,gp_tklock);

}

static DBusHandlerResult
tklock_dbus_filter(DBusConnection *connection, DBusMessage *message, void *user_data)
{
  const char *value;

  DEBUG_FN;

  if (dbus_message_is_signal (message,
                              MCE_SIGNAL_IF,
                              MCE_DISPLAY_SIG))
  {
    if (dbus_message_get_args (message, NULL,
                               DBUS_TYPE_STRING, &value,
                               DBUS_TYPE_INVALID))
      {
        SYSLOG_DEBUG("%s",value);
        if(strcmp(value, MCE_DISPLAY_OFF_STRING) == 0)
        {
          plugin_data->display_off = TRUE;
          remove_tklock_timeout();
        }
        else
        {
          tklock_destroy_hamm_window();
          plugin_data->display_off = FALSE;
        }
    }
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void plugin_close(system_ui_data *data)
{
  DEBUG_FN;

  if(plugin_data->data != data)
  {
    SYSLOG_ERROR("systemui context is inconsistent");
  }

  if(plugin_data->data)
  {
    remove_handler("tklock_open", plugin_data->data);
    remove_handler("tklock_close", plugin_data->data);
  }

  gp_tklock_destroy_lock(plugin_data->gp_tklock);
  visual_tklock_destroy_lock(plugin_data->vtklock);

  g_slice_free(tklock_plugin_data,plugin_data);

  plugin_data = NULL;

  closelog();
  /* FIXME dbus_bus_remove_match is missing */

  /*dbus_bus_remove_match(
    sui->system_bus,
    "type='signal',interface='com.nokia.mce.signal',path='/com/nokia/mce/signal'",
    &error);*/

}

static vtklock_t*
visual_tklock_new(tklock *gp_tklock)
{
  vtklock_t *vtklock;

  DEBUG_FN;

  vtklock = g_slice_alloc0(sizeof(vtklock_t));

  if(!vtklock)
  {
    SYSLOG_ERROR("failed to allocate memory");
    return NULL;
  }

  g_assert( gp_tklock->systemui_conn != NULL );

  vtklock->systemui_conn = gp_tklock->systemui_conn;
  visual_tklock_create_view_whimsy(vtklock);

  vtklock->priority = 290;
  vtklock->update_date_time_cb_tag = 0;

  return vtklock;
}

static void
visual_tklock_destroy_lock(vtklock_t *vtklock)
{
  DEBUG_FN;

  if(!vtklock)
    return;

  if(vtklock->update_date_time_cb_tag)
  {
    g_source_remove(vtklock->update_date_time_cb_tag);
    vtklock->update_date_time_cb_tag = 0;
  }

  ipm_hide_window(vtklock->window);
  gtk_widget_unrealize(vtklock->window);
  gtk_widget_destroy(vtklock->window);

  vtklock->slider_adjustment = NULL;
  vtklock->window = NULL;
  vtklock->ts.date_label = NULL;
  vtklock->ts.time_label = NULL;
  vtklock->slider = NULL;
}

static void
visual_tklock_destroy(vtklock_t *vtklock)
{
  DEBUG_FN;

  if(!vtklock)
    return;

  visual_tklock_destroy_lock(vtklock);

  g_slice_free(vtklock_t, vtklock);
}

gboolean vtklock_reset_slider_position(vtklock_t *vtklock)
{
  DEBUG_FN;

  g_assert(vtklock != NULL);
  g_assert(vtklock->slider != NULL && GTK_IS_RANGE(vtklock->slider));

  vtklock->slider_value = 3.0;
  vtklock->slider_status = 1;
  gtk_range_set_value(GTK_RANGE(vtklock->slider), vtklock->slider_value);

  return TRUE;
}

static void
set_gdk_property(GtkWidget *widget, GdkAtom property, gboolean value)
{
  if(GTK_WIDGET_REALIZED(widget))
  {
    gdk_property_change(widget->window,
                        property,
                        gdk_x11_xatom_to_atom(XA_CARDINAL),
                        32,
                        GDK_PROP_MODE_REPLACE,
                        (const guchar*)&value,
                        1
                        );
  }
}
static void
visual_tklock_set_hildon_flags(GtkWidget *window, gboolean portrait)
{
  DEBUG_FN;

  g_assert(window);
  /* FIXME */
  set_gdk_property(window, gdk_atom_intern_static_string("_HILDON_WM_ACTION_NO_TRANSITIONS"), TRUE);
  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
  hildon_gtk_window_set_do_not_disturb(GTK_WINDOW(window), TRUE);
  set_gdk_property(window, gdk_atom_intern_static_string("_HILDON_PORTRAIT_MODE_SUPPORT"), TRUE);
}


static void
visual_tklock_create_view_whimsy(vtklock_t *vtklock)
{
  gboolean portrait;
  GtkWidget *window_align;

  DEBUG_FN;

  if(vtklock->window)
    return;


  portrait = gdk_screen_height() > gdk_screen_width()?TRUE:FALSE;

  vtklock->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(vtklock->window), "visual_tklock");
  gtk_window_set_decorated(GTK_WINDOW(vtklock->window), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW(vtklock->window), TRUE);

  GdkColor color = {0, 0, 0, 128};
  gtk_widget_modify_bg(vtklock->window, GTK_STATE_NORMAL, &color);

  if(portrait)
  {
    window_align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(window_align), 8, 24, 1, 0);
  }
  else
  {
    window_align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(window_align), 0, 0, 0, 16);
  }

  gtk_container_add(GTK_CONTAINER(vtklock->window), window_align);

  g_signal_connect_data(vtklock->window, "key-press-event", G_CALLBACK(vtklock_key_cb), vtklock, NULL, 0);
  g_signal_connect_data(vtklock->window, "key-release-event", G_CALLBACK(vtklock_key_cb), vtklock, NULL, 0);

  gtk_widget_show_all(window_align);

  gtk_widget_realize(vtklock->window);

  visual_tklock_set_hildon_flags(vtklock->window, portrait);
  gtk_widget_show_all(vtklock->window);
}

static gboolean
tklock_timeout_cb(gpointer user_data)
{
  DEBUG_FN;

  plugin_data->tklock_timeout_tag = 0;

  if(!plugin_data->display_off)
  {
    tklock_destroy_hamm_window();

    if(plugin_data)
    {
      if(plugin_data->gp_tklock && !plugin_data->gp_tklock->window_hidden)
        gp_tklock_destroy_lock(plugin_data->gp_tklock);

      if(plugin_data->vtklock)
        visual_tklock_destroy_lock(plugin_data->vtklock);

      systemui_free_callback(&system_ui_callback);
    }
  }

  return FALSE;
}

static void
visual_tklock_present_view(vtklock_t *vtklock)
{
  GtkWidget *grab;

  DEBUG_FN;

  g_assert(vtklock != NULL);


  grab =  gtk_grab_get_current();
  if(!grab)
  {
    grab = vtklock->window;
    gtk_grab_add(grab);
  }

  gtk_widget_realize(vtklock->window);
  gdk_flush();
  ipm_show_window(vtklock->window, vtklock->priority);
  gdk_window_invalidate_rect(vtklock->window->window, NULL, TRUE);
  gdk_window_process_all_updates();
  gdk_flush();
}

static void
vtklock_unlock_handler()
{
  DEBUG_FN;

  systemui_do_callback( plugin_data->data, &system_ui_callback, 1);
}

static void
visual_tklock_set_unlock_handler(vtklock_t *vtklock, void(*unlock_handler)())
{
  DEBUG_FN;

  g_assert(vtklock != NULL);
  vtklock->unlock_handler = unlock_handler;
}

static gboolean
vtklock_key_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  vtklock_t *vtklock = (vtklock_t *)user_data;

  DEBUG_FN;

  g_assert(vtklock != NULL && vtklock->window != NULL && GTK_WIDGET_MAPPED(vtklock->window));
  return TRUE;
}

static void
remove_tklock_timeout()
{
  if(plugin_data->tklock_timeout_tag)
  {
    g_source_remove(plugin_data->tklock_timeout_tag);
    plugin_data->tklock_timeout_tag = 0;
  }
}
