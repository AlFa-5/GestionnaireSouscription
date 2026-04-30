#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QColor>
#include <QSize>
#include <QList>
#include <QPushButton>
#include <QHBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_lineEdit_2_textChanged(const QString &arg1);
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

    // Pagination
    QPushButton  *m_btnPrev    = nullptr;
    QPushButton  *m_btnNext    = nullptr;
    QHBoxLayout  *m_pageLayout = nullptr;
    int           m_currentPage = 0;
    QList<int>    m_lignesFiltrees;

    //Méthode
    QIcon colorizeIcon(const QString &svgPath, const QColor &color, const QSize &size);
    void  fichier();
    void  sauvegarderFichier();
    void  ajouterLigne(const QString &nom);
    void  appliquerFiltreEtPagination();
    void  afficherPage(int page);
    void  mettreAJourPagination();
    int   rowsPerPage();
};

#endif // MAINWINDOW_H
