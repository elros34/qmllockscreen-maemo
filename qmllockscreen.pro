TEMPLATE = subdirs
SUBDIRS  = src qmllockscreen-settings

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/postinst \
    qtc_packaging/debian_fremantle/postrm \
    README.md
