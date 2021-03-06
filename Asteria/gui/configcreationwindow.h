#ifndef CONFIGCREATIONWINDOW_H
#define CONFIGCREATIONWINDOW_H

#include <QDialog>

class QDialogButtonBox;
class QTabWidget;
class QLineEdit;
class AsteriaState;
class ConfigStore;
class ConfigParameterFamilyTab;

class ConfigCreationWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigCreationWindow(QWidget *parent = 0, AsteriaState * state = 0);
    ~ConfigCreationWindow();

private:

    AsteriaState *state;

    ConfigStore *store;
    ConfigParameterFamilyTab ** tabs;

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

signals:
    void ok();
    void cancel();

public slots:

    void loadClicked();
    void saveClicked();
    void okClicked();
    void cancelClicked();

};

#endif // CONFIGCREATIONWINDOW_H
