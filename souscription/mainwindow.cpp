#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>

// ─────────────────────────────────────────────
//  Utilitaire : colorise un SVG
// ─────────────────────────────────────────────
QIcon MainWindow::colorizeIcon(const QString &svgPath, const QColor &color, const QSize &size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QSvgRenderer renderer(svgPath);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}

// ─────────────────────────────────────────────
//  Constructeur
// ─────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Table
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //pour éviter d'éditer le tableau sans ciquer sur le boutton éditer
    ui->tableWidget->verticalHeader()->setVisible(false);    //effacer les numéros de ligne
    ui->tableWidget->setColumnCount(2);    //nombre de colonne
    ui->tableWidget->setHorizontalHeaderLabels({"Nom", "Action"});    //titre de chaque colonne
    ui->tableWidget->setColumnWidth(0, 512);    //taille fixe de colonne 1 
    ui->tableWidget->setColumnWidth(1, 158);    //taille fixe de colonne 2

    // Champs texte, placeholder
    ui->lineEdit->setPlaceholderText("Entrer un nom...");
    ui->lineEdit_2->setPlaceholderText("Recherche...");

    // ComboBox, selection, placeholder
    ui->comboBox->addItem("Croissant");
    ui->comboBox->addItem("Décroissant");
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->setPlaceholderText("Filtrer...");

    // Icônes boutons principaux
    ui->pushButton->setIcon(colorizeIcon(":/svg/ajouter.svg", QColor("#27AE60"), QSize(24, 24)));
    ui->pushButton->setIconSize(QSize(24, 24));

    ui->pushButton_2->setIcon(colorizeIcon(":/svg/actualiser.svg", QColor("#4A90D9"), QSize(24, 24)));
    ui->pushButton_2->setIconSize(QSize(24, 24));

    // Icône recherche dans le lineEdit
    QAction *searchAction = new QAction(
        colorizeIcon(":/svg/rechercher.svg", QColor("#4A90D9"), QSize(24, 24)), "", this);
    ui->lineEdit_2->addAction(searchAction, QLineEdit::LeadingPosition);

    // Pagination
    m_btnPrev = new QPushButton("‹");
    m_btnNext = new QPushButton("›");
    m_btnPrev->setFixedSize(30, 30);
    m_btnNext->setFixedSize(30, 30);

    m_pageLayout = qobject_cast<QHBoxLayout*>(ui->widget->layout());
    if (!m_pageLayout)
    {
        m_pageLayout = new QHBoxLayout(ui->widget);
        m_pageLayout->setAlignment(Qt::AlignCenter);
        m_pageLayout->setSpacing(4);
        m_pageLayout->setContentsMargins(0, 4, 0, 4);
    }

    connect(m_btnPrev, &QPushButton::clicked, this, [=]() {
        afficherPage(m_currentPage - 1);
    });
    connect(m_btnNext, &QPushButton::clicked, this, [=]() {
        afficherPage(m_currentPage + 1);
    });

    // Chargement initial
    fichier();
}

// ─────────────────────────────────────────────
//  Destructeur
// ─────────────────────────────────────────────
MainWindow::~MainWindow()
{
    delete ui;
}

// ─────────────────────────────────────────────
//  Chargement depuis bdd.csv
// ─────────────────────────────────────────────
void MainWindow::fichier()
{
    QFile file("bdd.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        appliquerFiltreEtPagination();
        return;
    }

    QTextStream in(&file);
    ui->tableWidget->setRowCount(0);

    while (!in.atEnd())
    {
        QString nom = in.readLine().trimmed();
        if (nom.isEmpty()) continue;
        ajouterLigne(nom);
    }

    appliquerFiltreEtPagination();
}

// ─────────────────────────────────────────────
//  Sauvegarde complète dans bdd.csv
// ─────────────────────────────────────────────
void MainWindow::sauvegarderFichier()
{
    QFile file("bdd.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;

    QTextStream out(&file);
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        if (item && !item->text().isEmpty())
            out << item->text() << "\n";
    }
}

// ─────────────────────────────────────────────
//  Ajoute une ligne dans la table
// ─────────────────────────────────────────────
void MainWindow::ajouterLigne(const QString &nom)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(nom));

    QWidget *cellWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(cellWidget);

    QPushButton *btnEdit   = new QPushButton();
    QPushButton *btnDelete = new QPushButton();

    btnEdit->setIcon(colorizeIcon(":/svg/modifier.svg", QColor("#4A90D9"), QSize(20, 20)));
    btnEdit->setIconSize(QSize(20, 20));
    btnEdit->setFixedSize(32, 32);
    btnEdit->setToolTip("Modifier");
    btnEdit->setFlat(true);

    btnDelete->setIcon(colorizeIcon(":/svg/supprimer.svg", QColor("#E74C3C"), QSize(20, 20)));
    btnDelete->setIconSize(QSize(20, 20));
    btnDelete->setFixedSize(32, 32);
    btnDelete->setToolTip("Supprimer");
    btnDelete->setFlat(true);

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(4);
    cellWidget->setLayout(layout);

    ui->tableWidget->setCellWidget(row, 1, cellWidget);

    // Suppression
    connect(btnDelete, &QPushButton::clicked, this, [=]() {
        QMessageBox msg(this);
        msg.setWindowTitle("Confirmation");
        msg.setText("Voulez-vous vraiment supprimer cette entrée ?");
        msg.setStandardButtons(QMessageBox::NoButton);
        QPushButton *btnOui = msg.addButton("Oui", QMessageBox::YesRole);
        QPushButton *btnNon = msg.addButton("Non", QMessageBox::NoRole);
        msg.setIcon(QMessageBox::Critical);
        msg.setDefaultButton(btnNon);

        if (msg.exec() == -1 || msg.clickedButton() != btnOui) return;

        for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
        {
            if (ui->tableWidget->cellWidget(r, 1) == cellWidget)
            {
                ui->tableWidget->removeRow(r);
                sauvegarderFichier();
                appliquerFiltreEtPagination();
                break;
            }
        }
    });

    // Modification
    connect(btnEdit, &QPushButton::clicked, this, [=]() {
        for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
        {
            if (ui->tableWidget->cellWidget(r, 1) == cellWidget)
            {
                QTableWidgetItem *item = ui->tableWidget->item(r, 0);
                if (!item) break;

                bool ok = false;
                QString newNom = QInputDialog::getText(
                    this, "Modifier", "Nouveau nom :",
                    QLineEdit::Normal, item->text(), &ok);

                if (ok && !newNom.trimmed().isEmpty())
                {
                    item->setText(newNom.trimmed());
                    sauvegarderFichier();
                }
                break;
            }
        }
    });
}

// ─────────────────────────────────────────────
//  Filtre recherche + recalcul pagination
// ─────────────────────────────────────────────
void MainWindow::appliquerFiltreEtPagination()
{
    QString recherche = ui->lineEdit_2->text().trimmed().toLower();

    m_lignesFiltrees.clear();
    for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
    {
        QTableWidgetItem *item = ui->tableWidget->item(r, 0);
        if (item && (recherche.isEmpty() || item->text().toLower().contains(recherche)))
            m_lignesFiltrees.append(r);
    }

    afficherPage(0);
}

// ─────────────────────────────────────────────
//  Affiche la page demandée
// ─────────────────────────────────────────────
void MainWindow::afficherPage(int page)
{
    if (!m_pageLayout) return;

    int total      = m_lignesFiltrees.size();
    int perPage    = rowsPerPage();
    int totalPages = qMax(1, (total + perPage - 1) / perPage);

    m_currentPage = qBound(0, page, totalPages - 1);

    int debut = m_currentPage * perPage;
    int fin   = debut + perPage;

    // Cache toutes les lignes
    for (int r = 0; r < ui->tableWidget->rowCount(); ++r)
        ui->tableWidget->setRowHidden(r, true);

    // Affiche uniquement la page courante parmi les filtrées
    for (int i = debut; i < qMin(fin, total); ++i)
        ui->tableWidget->setRowHidden(m_lignesFiltrees[i], false);

    mettreAJourPagination();
}

// ─────────────────────────────────────────────
//  Met à jour les boutons de pagination
// ─────────────────────────────────────────────
void MainWindow::mettreAJourPagination()
{
    if (!m_pageLayout) return;

    int total      = m_lignesFiltrees.size();
    int perPage    = rowsPerPage();
    int totalPages = qMax(1, (total + perPage - 1) / perPage);

    // Vide les anciens boutons sans détruire m_btnPrev / m_btnNext
    QLayoutItem *child;
    while ((child = m_pageLayout->takeAt(0)) != nullptr)
    {
        QWidget *w = child->widget();
        delete child;
        if (w && w != m_btnPrev && w != m_btnNext)
            delete w;
    }

    // Bouton précédent
    m_pageLayout->addWidget(m_btnPrev);
    m_btnPrev->setEnabled(m_currentPage > 0);

    // Numéros de pages
    for (int i = 0; i < totalPages; ++i)
    {
        QPushButton *btn = new QPushButton(QString::number(i + 1));
        btn->setFixedSize(30, 30);
        btn->setEnabled(i != m_currentPage);
        btn->setFlat(i == m_currentPage);
        connect(btn, &QPushButton::clicked, this, [=]() {
            afficherPage(i);
        });
        m_pageLayout->addWidget(btn);
    }

    // Bouton suivant
    m_pageLayout->addWidget(m_btnNext);
    m_btnNext->setEnabled(m_currentPage < totalPages - 1);
}

// ─────────────────────────────────────────────
//  Nombre de lignes par page
// ─────────────────────────────────────────────
int MainWindow::rowsPerPage()
{
    return 20;
}

// ─────────────────────────────────────────────
//  Slots
// ─────────────────────────────────────────────
void MainWindow::on_pushButton_clicked()
{
    QString nom = ui->lineEdit->text().trimmed();
    if (nom.isEmpty()) return;

    ajouterLigne(nom);
    sauvegarderFichier();

    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    appliquerFiltreEtPagination();
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->comboBox->setCurrentIndex(-1);
    ui->lineEdit_2->clear();
    fichier();
}

void MainWindow::on_lineEdit_2_textChanged(const QString &)
{
    appliquerFiltreEtPagination();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if (index == 0)
        ui->tableWidget->sortItems(0, Qt::AscendingOrder);
    else if (index == 1)
        ui->tableWidget->sortItems(0, Qt::DescendingOrder);

    appliquerFiltreEtPagination();
}
