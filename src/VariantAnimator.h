#ifndef VARIANTANIMATOR_H
#define VARIANTANIMATOR_H

#include <QVariantAnimation>

class VariantAnimator : public QVariantAnimation
{
    Q_OBJECT
public:
    VariantAnimator(){};

    virtual ~VariantAnimator(){};

    void updateCurrentValue(const QVariant &value) {
        emit valueChanged(value);
    }
};

#endif // VARIANTANIMATOR_H

