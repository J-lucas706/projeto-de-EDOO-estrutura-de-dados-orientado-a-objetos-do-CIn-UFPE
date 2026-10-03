// ============================================================
//  Front end (Qt) do Sistema de Alocacao - CIn UFPE
//  Reaproveita as classes Sala, SalaTeorica, Laboratorio e Reserva
//  e le/grava os MESMOS arquivos salas.csv e reservas.csv do terminal.
// ============================================================

#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStatusBar>
#include <QPalette>
#include <QCollator>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "salas.h"   // do projeto original (pasta include/)


//  Camada de dados: guarda as salas e conversa com os CSVs

class Repositorio {
public:
    std::map<std::string, Sala*> salas;   // ordenado pelo codigo
    QString arqSalas;
    QString arqReservas;

    ~Repositorio() {
        for (auto& par : salas) delete par.second;
    }

    void carregar(const QString& pasta) {
        arqSalas = pasta + "/salas.csv";
        arqReservas = pasta + "/reservas.csv";

        // ---- salas ----
        QFile fs(arqSalas);
        if (fs.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&fs);
            while (!in.atEnd()) {
                QString linha = in.readLine().trimmed();
                if (linha.isEmpty() || linha.startsWith('#')) continue;

                QStringList p = linha.split(',');
                if (p.size() < 4) continue;

                bool ok = false;
                int cap = p[2].toInt(&ok);
                if (!ok) continue;

                std::string cod = p[1].trimmed().toStdString();
                if (salas.count(cod)) continue;

                if (p[0] == "T") {
                    salas[cod] = new SalaTeorica(cod, cap, p[3].toInt() == 1);
                } else if (p[0] == "L" && p.size() >= 5) {
                    salas[cod] = new Laboratorio(cod, cap, p[3].toStdString(), p[4].toInt());
                }
            }
        }

        // ---- reservas ----
        QFile fr(arqReservas);
        if (fr.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&fr);
            while (!in.atEnd()) {
                QString linha = in.readLine().trimmed();
                if (linha.isEmpty() || linha.startsWith('#')) continue;

                QStringList p = linha.split(',');
                if (p.size() < 4) continue;

                auto it = salas.find(p[0].trimmed().toStdString());
                if (it == salas.end()) continue;

                try {
                    it->second->adicionarReserva(
                        Reserva(p[1].toStdString(), p[2].toStdString(), p[3].toStdString()));
                } catch (...) {
                    // linha com horario invalido: ignora
                }
            }
        }
    }

    bool salvar() const {
        QFile fs(arqSalas);
        if (!fs.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
        {
            QTextStream o(&fs);
            o << "# tipo,codigo,capacidade,extra1,extra2\n";
            o << "# T = teorica (extra1: projetor 1/0) | L = laboratorio (extra1: Hardware/Software, extra2: qtd computadores)\n";
            for (const auto& par : salas)
                o << QString::fromStdString(par.second->paraCsv()) << "\n";
        }
        fs.close();

        QFile fr(arqReservas);
        if (!fr.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
        {
            QTextStream o(&fr);
            o << "# codigo_sala,dia,hora_inicio,hora_fim\n";
            for (const auto& par : salas)
                for (const auto& r : par.second->getReservas())
                    o << QString::fromStdString(r.paraCsv(par.first)) << "\n";
        }
        fr.close();
        return true;
    }
};

//  Estilo (tema)

static const char* ESTILO = R"(
* { font-family: "Open Sans", "Segoe UI", sans-serif; }
QMainWindow, QWidget#raiz { background: #f4f4f4; }

QFrame#cabecalho {
    background: white; border: 1px solid #e3e3e3; border-bottom: 4px solid #D6002B;
    border-radius: 10px;
}
QLabel#titulo { font-size: 27px; font-weight: 300; color: #D6002B; background: transparent; }
QLabel#subHeader { font-size: 13px; color: #6b6b6b; background: transparent; }
QLabel#dataHeader { font-size: 13px; font-weight: 600; color: #D6002B; background: transparent; }
QLabel#cartaoTitulo { font-size: 12px; color: #6b6b6b; background: transparent; }
QLabel#info { color: #2b2b2b; font-weight: bold; }
QLabel#dica { color: #8a8a8a; font-size: 12px; }

QTabWidget::pane { border: none; }
QTabBar::tab {
    padding: 12px 26px; margin-right: 2px; background: transparent;
    color: #D6002B; font-size: 14px; font-weight: 600;
    border-bottom: 3px solid transparent;
}
QTabBar::tab:hover { background: #fbe9ec; }
QTabBar::tab:selected { color: #A80021; border-bottom: 3px solid #D6002B; }

QFrame#card {
    background: #ffffff; border: 1px solid #e3e3e3; border-bottom: 4px solid #D6002B;
    border-radius: 10px;
}
QFrame#card QLabel { color: #2b2b2b; font-weight: 600; background: transparent; }

QTableWidget {
    font-size: 13px; color: #2b2b2b;
    background: #ffffff; alternate-background-color: #fafafa;
    border: 1px solid #e3e3e3; border-bottom: 4px solid #D6002B; border-radius: 8px;
    gridline-color: #eeeeee;
    selection-background-color: #fbd5dc; selection-color: #A80021;
}
QHeaderView::section {
    background: #D6002B; color: white; padding: 9px; border: none; font-weight: bold;
}
QHeaderView::section:hover { background: #B80025; }

QLineEdit, QComboBox, QSpinBox, QDateEdit, QTimeEdit {
    color: #2b2b2b; padding: 7px; border: 1px solid #c9c9c9; border-radius: 6px;
    background: white; min-height: 20px; placeholder-text-color: #8a8a8a;
    selection-background-color: #D6002B; selection-color: white;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDateEdit:focus, QTimeEdit:focus {
    border: 2px solid #D6002B;
}
QComboBox QAbstractItemView {
    background: white; color: #2b2b2b;
    selection-background-color: #fbd5dc; selection-color: #A80021;
}
QListWidget {
    background: white; color: #2b2b2b; border: 1px solid #e3e3e3;
    border-bottom: 4px solid #D6002B; border-radius: 8px; font-size: 13px;
}
QListWidget::item { padding: 8px; }
QListWidget::item:selected { background: #fbd5dc; color: #A80021; }
QCheckBox { color: #2b2b2b; }

QPushButton {
    padding: 8px 18px; border-radius: 6px; border: 1px solid #c9c9c9;
    background: white; color: #2b2b2b; font-weight: bold;
}
QPushButton:hover { border: 1px solid #D6002B; color: #D6002B; background: white; }
QPushButton:disabled { color: #b0b0b0; }
QPushButton#primario { background: #D6002B; color: white; border: none; }
QPushButton#primario:hover { background: #B80025; color: white; }
QPushButton#perigo { color: #D6002B; border: 1px solid #D6002B; }
QPushButton#perigo:hover { background: #fbe9ec; }

QStatusBar { color: #2b2b2b; }
QDialog, QMessageBox { background: #f4f4f4; }
QDialog QLabel, QMessageBox QLabel { color: #2b2b2b; }
)";


//  Funcoes auxiliares

static void descrever(const Sala* s, QString& tipo, QString& detalhe) {
    if (auto* t = dynamic_cast<const SalaTeorica*>(s)) {
        tipo = "Teórica";
        detalhe = t->getTemProjetor() ? "Com projetor" : "Sem projetor";
    } else if (auto* l = dynamic_cast<const Laboratorio*>(s)) {
        tipo = "Laboratório";
        detalhe = QString("%1 · %2 computadores")
                      .arg(QString::fromStdString(l->getTipoLab()))
                      .arg(l->getQtdComputadores());
    }
}

// Copia a sala (mesmo tipo e dados), pulando a reserva de indice "ignorar"
// Usado para cancelar uma reserva sem precisar alterar a classe Sala
static Sala* clonarSemReserva(const Sala* s, int ignorar) {
    Sala* n = nullptr;
    if (auto* t = dynamic_cast<const SalaTeorica*>(s)) {
        n = new SalaTeorica(t->getCodigo(), t->getCapacidade(), t->getTemProjetor());
    } else if (auto* l = dynamic_cast<const Laboratorio*>(s)) {
        n = new Laboratorio(l->getCodigo(), l->getCapacidade(), l->getTipoLab(),
                            l->getQtdComputadores());
    }
    if (!n) return nullptr;
    const auto& rs = s->getReservas();
    for (size_t i = 0; i < rs.size(); i++)
        if (static_cast<int>(i) != ignorar) n->adicionarReserva(rs[i]);
    return n;
}

//  Janela principal

class Janela : public QMainWindow {
public:
    explicit Janela(const QString& pasta) {
        repo.carregar(pasta);

        setWindowTitle("Sistema de Alocação - CIn UFPE");
        resize(1020, 700);

        auto* raiz = new QWidget;
        raiz->setObjectName("raiz");
        auto* lay = new QVBoxLayout(raiz);
        lay->setContentsMargins(24, 18, 24, 12);
        lay->setSpacing(14);

        // ---- cabecalho ----
        auto* cab = new QFrame;
        cab->setObjectName("cabecalho");
        auto* cabLay = new QHBoxLayout(cab);
        cabLay->setContentsMargins(26, 18, 26, 18);
        auto* esq = new QVBoxLayout;
        auto* titulo = new QLabel("Alocação de Salas no CIn-UFPE");
        titulo->setObjectName("titulo");
        auto* sub = new QLabel("Gerencie salas, laboratórios e reservas");
        sub->setObjectName("subHeader");
        esq->addWidget(titulo);
        esq->addWidget(sub);
        cabLay->addLayout(esq, 1);
        auto* lblData = new QLabel(QLocale(QLocale::Portuguese, QLocale::Brazil)
                                       .toString(QDate::currentDate(), "dddd, d 'de' MMMM 'de' yyyy"));
        lblData->setObjectName("dataHeader");
        cabLay->addWidget(lblData, 0, Qt::AlignRight | Qt::AlignVCenter);
        lay->addWidget(cab);

        // ---- abas ----
        auto* abas = new QTabWidget;
        abas->addTab(criarAbaSalas(), "  SALAS  ");
        abas->addTab(criarAbaReservas(), "  RESERVAS  ");
        lay->addWidget(abas, 1);

        setCentralWidget(raiz);
        statusBar()->showMessage(QString("%1 sala(s) carregada(s).").arg(repo.salas.size()));

        atualizarTudo();
    }

private:
    Repositorio repo;

    // aba salas
    QTableWidget* tabSalas = nullptr;
    QLineEdit* busca = nullptr;
    QComboBox* filtroTipo = nullptr;
    QLabel* lblTotal = nullptr;
    QLabel* lblTeo = nullptr;
    QLabel* lblLab = nullptr;
    QLabel* lblRes = nullptr;

    // aba reservas
    QComboBox* comboSala = nullptr;
    QDateEdit* data = nullptr;
    QTimeEdit* hIni = nullptr;
    QTimeEdit* hFim = nullptr;
    QCheckBox* chkTodas = nullptr;
    QTableWidget* tabRes = nullptr;
    QLabel* lblInfo = nullptr;
    int ordCol = 1;                              // coluna usada na ordenacao (1 = Dia)
    Qt::SortOrder ordDir = Qt::AscendingOrder;   // crescente ou decrescente
    int salCol = 0;                              // ordenacao da tabela de salas (0 = Codigo)
    Qt::SortOrder salDir = Qt::AscendingOrder;

    // ---------------- construcao das abas ----------------
    QFrame* criarCartao(const QString& titulo, QLabel*& valor, const QString& cor) {
        auto* c = new QFrame;
        c->setObjectName("cartao");
        c->setStyleSheet("QFrame#cartao { background: white; border: 1px solid #e3e3e3;"
                         " border-bottom: 4px solid #D6002B; border-radius: 10px; }");
        auto* l = new QVBoxLayout(c);
        l->setContentsMargins(16, 10, 16, 10);
        l->setSpacing(0);
        auto* t = new QLabel(titulo);
        t->setObjectName("cartaoTitulo");
        valor = new QLabel("0");
        valor->setStyleSheet("font-size: 30px; font-weight: bold; background: transparent; color: " + cor + ";");
        l->addWidget(t);
        l->addWidget(valor);
        return c;
    }

    QWidget* criarAbaSalas() {
        auto* w = new QWidget;
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(0, 12, 0, 0);
        lay->setSpacing(12);

        // painel de resumo
        auto* resumo = new QHBoxLayout;
        resumo->setSpacing(12);
        resumo->addWidget(criarCartao("Total de salas", lblTotal, "#D6002B"));
        resumo->addWidget(criarCartao("Salas teóricas", lblTeo, "#2b2b2b"));
        resumo->addWidget(criarCartao("Laboratórios", lblLab, "#2b2b2b"));
        resumo->addWidget(criarCartao("Reservas feitas", lblRes, "#D6002B"));
        lay->addLayout(resumo);

        // barra de ferramentas
        auto* topo = new QHBoxLayout;
        busca = new QLineEdit;
        busca->setPlaceholderText("Buscar por código, tipo ou detalhes...");
        busca->setClearButtonEnabled(true);
        filtroTipo = new QComboBox;
        filtroTipo->addItems(QStringList{"Todos os tipos", "Teóricas", "Laboratórios"});
        auto* btnAdd = new QPushButton("+ Adicionar sala");
        btnAdd->setObjectName("primario");
        auto* btnEdit = new QPushButton("Editar");
        auto* btnRem = new QPushButton("Remover");
        btnRem->setObjectName("perigo");
        topo->addWidget(busca, 1);
        topo->addWidget(filtroTipo);
        topo->addWidget(btnAdd);
        topo->addWidget(btnEdit);
        topo->addWidget(btnRem);
        lay->addLayout(topo);

        tabSalas = new QTableWidget(0, 5);
        tabSalas->setHorizontalHeaderLabels(
            QStringList{"Código", "Tipo", "Capacidade", "Detalhes", "Reservas"});
        configurarTabela(tabSalas);
        tabSalas->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        tabSalas->horizontalHeader()->setSectionsClickable(true);
        tabSalas->horizontalHeader()->setCursor(Qt::PointingHandCursor);
        tabSalas->horizontalHeader()->setToolTip("Clique para ordenar. Clique de novo para inverter.");
        connect(tabSalas->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int col) {
            if (col == salCol) {
                salDir = (salDir == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
            } else {
                salCol = col;
                salDir = Qt::AscendingOrder;
            }
            atualizarSalas();
        });
        lay->addWidget(tabSalas, 1);

        auto* dica = new QLabel("Dicas: dê dois cliques em uma sala para editá-la. Clique no título de uma coluna para ordenar.");
        dica->setObjectName("dica");
        lay->addWidget(dica);

        connect(busca, &QLineEdit::textChanged, this, [this] { atualizarSalas(); });
        connect(filtroTipo, &QComboBox::currentIndexChanged, this, [this] { atualizarSalas(); });
        connect(btnAdd, &QPushButton::clicked, this, [this] { dialogoSala(nullptr); });
        connect(btnEdit, &QPushButton::clicked, this, [this] { editarSelecionada(); });
        connect(btnRem, &QPushButton::clicked, this, [this] { removerSala(); });
        connect(tabSalas, &QTableWidget::cellDoubleClicked, this,
                [this](int, int) { editarSelecionada(); });
        return w;
    }

    QWidget* criarAbaReservas() {
        auto* w = new QWidget;
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(0, 12, 0, 0);
        lay->setSpacing(12);

        auto* card = new QFrame;
        card->setObjectName("card");
        auto* cardLay = new QHBoxLayout(card);
        cardLay->setContentsMargins(16, 14, 16, 14);

        comboSala = new QComboBox;
        comboSala->setMinimumWidth(110);

        data = new QDateEdit(QDate::currentDate());
        data->setLocale(QLocale(QLocale::Portuguese, QLocale::Brazil));
        data->setCalendarPopup(true);
        data->setDisplayFormat("dd/MM/yyyy");

        hIni = new QTimeEdit(QTime(14, 0));
        hIni->setDisplayFormat("HH:mm");
        hFim = new QTimeEdit(QTime(16, 0));
        hFim->setDisplayFormat("HH:mm");

        auto* btnLivres = new QPushButton("Ver salas livres");
        auto* btnReservar = new QPushButton("Reservar");
        btnReservar->setObjectName("primario");

        cardLay->addWidget(new QLabel("Sala"));
        cardLay->addWidget(comboSala);
        cardLay->addSpacing(10);
        cardLay->addWidget(new QLabel("Dia"));
        cardLay->addWidget(data);
        cardLay->addSpacing(10);
        cardLay->addWidget(new QLabel("Início"));
        cardLay->addWidget(hIni);
        cardLay->addWidget(new QLabel("Fim"));
        cardLay->addWidget(hFim);
        cardLay->addStretch(1);
        cardLay->addWidget(btnLivres);
        cardLay->addWidget(btnReservar);
        lay->addWidget(card);

        auto* barra = new QHBoxLayout;
        lblInfo = new QLabel;
        lblInfo->setObjectName("info");
        chkTodas = new QCheckBox("Mostrar reservas de todas as salas");
        auto* btnCancelar = new QPushButton("Cancelar reserva selecionada");
        btnCancelar->setObjectName("perigo");
        barra->addWidget(lblInfo, 1);
        barra->addWidget(chkTodas);
        barra->addWidget(btnCancelar);
        lay->addLayout(barra);

        tabRes = new QTableWidget(0, 5);
        tabRes->setHorizontalHeaderLabels(QStringList{"Sala", "Dia", "Início", "Fim", "Situação"});
        configurarTabela(tabRes);
        tabRes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tabRes->horizontalHeader()->setSectionsClickable(true);
        tabRes->horizontalHeader()->setCursor(Qt::PointingHandCursor);
        tabRes->horizontalHeader()->setToolTip("Clique para ordenar. Clique de novo para inverter.");
        lay->addWidget(tabRes, 1);

        auto* dicaOrd = new QLabel("Dica: clique no título de uma coluna para ordenar "
                                   "(clique de novo para inverter a ordem).");
        dicaOrd->setObjectName("dica");
        lay->addWidget(dicaOrd);

        connect(tabRes->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int col) {
            if (col == ordCol) {
                ordDir = (ordDir == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
            } else {
                ordCol = col;
                ordDir = Qt::AscendingOrder;
            }
            atualizarReservas();
        });

        connect(comboSala, &QComboBox::currentIndexChanged, this, [this] { atualizarReservas(); });
        connect(chkTodas, &QCheckBox::toggled, this, [this] { atualizarReservas(); });
        connect(btnReservar, &QPushButton::clicked, this, [this] { reservar(); });
        connect(btnLivres, &QPushButton::clicked, this, [this] { salasLivres(); });
        connect(btnCancelar, &QPushButton::clicked, this, [this] { cancelarReserva(); });
        return w;
    }

    static void configurarTabela(QTableWidget* t) {
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setSelectionBehavior(QAbstractItemView::SelectRows);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->setAlternatingRowColors(true);
        t->verticalHeader()->setVisible(false);
        t->verticalHeader()->setDefaultSectionSize(36);
        t->horizontalHeader()->setStretchLastSection(true);
        t->setShowGrid(false);
    }

    bool confirmar(const QString& titulo, const QString& msg) {
        QMessageBox box(QMessageBox::Question, titulo, msg, QMessageBox::NoButton, this);
        auto* sim = box.addButton("Sim", QMessageBox::YesRole);
        box.addButton("Não", QMessageBox::NoRole);
        box.exec();
        return box.clickedButton() == sim;
    }

    // ---------------- atualizacao das telas ----------------
    void atualizarTudo() {
        atualizarSalas();
        atualizarResumo();
        atualizarCombo();
    }

    void atualizarResumo() {
        int teo = 0, lab = 0, res = 0;
        for (const auto& par : repo.salas) {
            if (dynamic_cast<SalaTeorica*>(par.second)) teo++;
            else lab++;
            res += static_cast<int>(par.second->getReservas().size());
        }
        lblTotal->setText(QString::number(repo.salas.size()));
        lblTeo->setText(QString::number(teo));
        lblLab->setText(QString::number(lab));
        lblRes->setText(QString::number(res));
    }

    void atualizarSalas() {
        QString filtro = busca->text().trimmed();
        int tipoFiltro = filtroTipo->currentIndex();   // 0 todos, 1 teoricas, 2 labs
        tabSalas->setRowCount(0);

        // seta (▲ crescente / ▼ decrescente) na coluna ordenada
        QStringList nomes{"Código", "Tipo", "Capacidade", "Detalhes", "Reservas"};
        nomes[salCol] += (salDir == Qt::AscendingOrder) ? "  ▲" : "  ▼";
        tabSalas->setHorizontalHeaderLabels(nomes);

        struct Linha {
            QString cod, tipo, det;
            bool ehTeorica;
            int cap;
            int nRes;
        };
        std::vector<Linha> linhas;

        for (const auto& par : repo.salas) {
            Sala* s = par.second;
            bool ehTeorica = dynamic_cast<SalaTeorica*>(s) != nullptr;
            if (tipoFiltro == 1 && !ehTeorica) continue;
            if (tipoFiltro == 2 && ehTeorica) continue;

            QString cod = QString::fromStdString(s->getCodigo());
            QString tipo, det;
            descrever(s, tipo, det);

            if (!filtro.isEmpty() &&
                !(cod + " " + tipo + " " + det).contains(filtro, Qt::CaseInsensitive))
                continue;

            linhas.push_back({cod, tipo, det, ehTeorica, s->getCapacidade(),
                              static_cast<int>(s->getReservas().size())});
        }

        // ordenacao: coluna e direcao escolhidas ao clicar no cabecalho
        QCollator colador;                       // "E2" vem antes de "E10"
        colador.setNumericMode(true);
        colador.setCaseSensitivity(Qt::CaseInsensitive);
        const int coluna = salCol;
        const bool cresc = (salDir == Qt::AscendingOrder);
        auto cmp = [](auto x, auto y) { return x < y ? -1 : (y < x ? 1 : 0); };

        std::sort(linhas.begin(), linhas.end(), [&](const Linha& a, const Linha& b) {
            int r = 0;
            switch (coluna) {
                case 0: r = colador.compare(a.cod, b.cod); break;
                case 1: r = cmp(a.ehTeorica ? 0 : 1, b.ehTeorica ? 0 : 1); break;
                case 2: r = cmp(a.cap, b.cap); break;
                case 3: r = colador.compare(a.det, b.det); break;
                case 4: r = cmp(a.nRes, b.nRes); break;
            }
            if (r != 0) return cresc ? (r < 0) : (r > 0);
            return colador.compare(a.cod, b.cod) < 0;   // desempate: codigo
        });

        for (const auto& l : linhas) {
            int r = tabSalas->rowCount();
            tabSalas->insertRow(r);

            auto* itCod = new QTableWidgetItem(l.cod);
            QFont f = itCod->font();
            f.setBold(true);
            itCod->setFont(f);

            auto* itTipo = new QTableWidgetItem(l.tipo);
            itTipo->setForeground(QColor(l.ehTeorica ? "#D6002B" : "#2b2b2b"));
            itTipo->setFont(f);

            auto* itCap = new QTableWidgetItem(QString::number(l.cap));
            itCap->setTextAlignment(Qt::AlignCenter);

            auto* itRes = new QTableWidgetItem(QString::number(l.nRes));
            itRes->setTextAlignment(Qt::AlignCenter);
            if (l.nRes > 0) {
                itRes->setForeground(QColor("#D6002B"));
                itRes->setFont(f);
            }

            tabSalas->setItem(r, 0, itCod);
            tabSalas->setItem(r, 1, itTipo);
            tabSalas->setItem(r, 2, itCap);
            tabSalas->setItem(r, 3, new QTableWidgetItem(l.det));
            tabSalas->setItem(r, 4, itRes);
        }
    }

    void atualizarCombo() {
        QString atual = comboSala->currentText();
        comboSala->blockSignals(true);
        comboSala->clear();
        for (const auto& par : repo.salas)
            comboSala->addItem(QString::fromStdString(par.first));
        int i = comboSala->findText(atual);
        if (i >= 0) comboSala->setCurrentIndex(i);
        comboSala->blockSignals(false);
        atualizarReservas();
    }

    void atualizarReservas() {
        tabRes->setRowCount(0);

        // mostra a seta (▲ crescente / ▼ decrescente) na coluna ordenada
        QStringList nomes{"Sala", "Dia", "Início", "Fim", "Situação"};
        nomes[ordCol] += (ordDir == Qt::AscendingOrder) ? "  ▲" : "  ▼";
        tabRes->setHorizontalHeaderLabels(nomes);

        const QDate hoje = QDate::currentDate();

        struct Linha {
            QString sala, dia, hi, hf;
            int idx;
            QDate dt;
            int minutos;   // inicio, em minutos
            int fimMin;    // fim, em minutos
            int rank;      // situacao: 0 = hoje, 1 = agendada, 2 = concluida
        };
        std::vector<Linha> linhas;

        bool todas = chkTodas->isChecked();
        QString atual = comboSala->currentText();

        for (const auto& par : repo.salas) {
            QString cod = QString::fromStdString(par.first);
            if (!todas && cod != atual) continue;
            const auto& rs = par.second->getReservas();
            for (size_t i = 0; i < rs.size(); i++) {
                QString dia = QString::fromStdString(rs[i].getDia());
                QDate dt = QDate::fromString(dia, "dd/MM/yyyy");
                int rank = 1;                                   // agendada
                if (dt.isValid() && dt == hoje) rank = 0;       // hoje
                else if (dt.isValid() && dt < hoje) rank = 2;   // concluida
                linhas.push_back({cod, dia,
                                  QString::fromStdString(rs[i].getHoraInicio()),
                                  QString::fromStdString(rs[i].getHoraFim()),
                                  static_cast<int>(i),
                                  dt,
                                  rs[i].getInicioMinutos(),
                                  rs[i].getFimMinutos(),
                                  rank});
            }
        }

        // ordenacao: coluna e direcao escolhidas ao clicar no cabecalho
        QCollator colador;                       // "E2" vem antes de "E10"
        colador.setNumericMode(true);
        colador.setCaseSensitivity(Qt::CaseInsensitive);
        const int coluna = ordCol;
        const bool cresc = (ordDir == Qt::AscendingOrder);
        auto cmp = [](auto x, auto y) { return x < y ? -1 : (y < x ? 1 : 0); };

        std::sort(linhas.begin(), linhas.end(), [&](const Linha& a, const Linha& b) {
            int r = 0;
            switch (coluna) {
                case 0: r = colador.compare(a.sala, b.sala); break;
                case 1: r = cmp(a.dt, b.dt); break;
                case 2: r = cmp(a.minutos, b.minutos); break;
                case 3: r = cmp(a.fimMin, b.fimMin); break;
                case 4: r = cmp(a.rank, b.rank); break;
            }
            if (r != 0) return cresc ? (r < 0) : (r > 0);

            // desempate (sempre crescente): data, horario de inicio, sala
            if (a.dt != b.dt) return a.dt < b.dt;
            if (a.minutos != b.minutos) return a.minutos < b.minutos;
            return colador.compare(a.sala, b.sala) < 0;
        });

        for (const auto& l : linhas) {
            int r = tabRes->rowCount();
            tabRes->insertRow(r);

            auto* itSala = new QTableWidgetItem(l.sala);
            itSala->setData(Qt::UserRole, l.idx);   // posicao da reserva dentro da sala
            QFont f = itSala->font();
            f.setBold(true);
            itSala->setFont(f);

            static const char* nomesSit[] = {"Hoje", "Agendada", "Concluída"};
            static const char* coresSit[] = {"#D6002B", "#2b2b2b", "#9a9a9a"};
            QString situacao = nomesSit[l.rank];
            QColor cor(coresSit[l.rank]);

            auto* itSit = new QTableWidgetItem(situacao);
            itSit->setForeground(cor);
            itSit->setFont(f);

            tabRes->setItem(r, 0, itSala);
            tabRes->setItem(r, 1, new QTableWidgetItem(l.dia));
            tabRes->setItem(r, 2, new QTableWidgetItem(l.hi));
            tabRes->setItem(r, 3, new QTableWidgetItem(l.hf));
            tabRes->setItem(r, 4, itSit);
        }

        if (todas)
            lblInfo->setText(QString("%1 reserva(s) no total").arg(linhas.size()));
        else if (comboSala->currentIndex() >= 0)
            lblInfo->setText(QString("Sala %1: %2 reserva(s)").arg(atual).arg(linhas.size()));
        else
            lblInfo->setText("Nenhuma sala cadastrada.");
    }

    // ---------------- salas: adicionar / editar / remover ----------------
    void editarSelecionada() {
        int row = tabSalas->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "Editar sala", "Selecione uma sala na tabela.");
            return;
        }
        auto it = repo.salas.find(tabSalas->item(row, 0)->text().toStdString());
        if (it != repo.salas.end()) dialogoSala(it->second);
    }

    // existente == nullptr  -> adicionar | existente != nullptr -> editar
    void dialogoSala(Sala* existente) {
        const bool editando = (existente != nullptr);

        QDialog d(this);
        d.setWindowTitle(editando ? "Editar sala" : "Adicionar sala");
        d.setMinimumWidth(360);

        auto* form = new QFormLayout;
        auto* tipo = new QComboBox;
        tipo->addItems(QStringList{"Teórica", "Laboratório"});

        auto* cod = new QLineEdit;
        cod->setPlaceholderText("Ex: E6");
        cod->setValidator(new QRegularExpressionValidator(
            QRegularExpression("[A-Za-z0-9_-]*"), cod));

        auto* cap = new QSpinBox;
        cap->setRange(1, 500);
        cap->setValue(30);

        auto* proj = new QCheckBox("Possui projetor");

        auto* lblTipoLab = new QLabel("Tipo do laboratório");
        auto* tipoLab = new QComboBox;
        tipoLab->addItems(QStringList{"Hardware", "Software"});

        auto* lblPcs = new QLabel("Computadores");
        auto* pcs = new QSpinBox;
        pcs->setRange(0, 300);
        pcs->setValue(20);

        form->addRow("Tipo", tipo);
        form->addRow("Código", cod);
        form->addRow("Capacidade", cap);
        form->addRow(proj);
        form->addRow(lblTipoLab, tipoLab);
        form->addRow(lblPcs, pcs);

        if (editando) {
            cod->setText(QString::fromStdString(existente->getCodigo()));
            cod->setEnabled(false);
            tipo->setEnabled(false);
            cap->setValue(existente->getCapacidade());
            if (auto* t = dynamic_cast<SalaTeorica*>(existente)) {
                tipo->setCurrentIndex(0);
                proj->setChecked(t->getTemProjetor());
            } else if (auto* l = dynamic_cast<Laboratorio*>(existente)) {
                tipo->setCurrentIndex(1);
                QString tl = QString::fromStdString(l->getTipoLab());
                if (tipoLab->findText(tl) < 0) tipoLab->addItem(tl);
                tipoLab->setCurrentText(tl);
                pcs->setValue(l->getQtdComputadores());
            }
        }

        auto mostrarCampos = [=] {
            bool lab = tipo->currentIndex() == 1;
            proj->setVisible(!lab);
            lblTipoLab->setVisible(lab);
            tipoLab->setVisible(lab);
            lblPcs->setVisible(lab);
            pcs->setVisible(lab);
        };
        connect(tipo, &QComboBox::currentIndexChanged, &d, mostrarCampos);
        mostrarCampos();

        auto* botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        botoes->button(QDialogButtonBox::Ok)->setText("Salvar");
        botoes->button(QDialogButtonBox::Ok)->setObjectName("primario");
        botoes->button(QDialogButtonBox::Cancel)->setText("Cancelar");
        connect(botoes, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        connect(botoes, &QDialogButtonBox::rejected, &d, &QDialog::reject);

        auto* lay = new QVBoxLayout(&d);
        lay->addLayout(form);
        lay->addWidget(botoes);

        if (d.exec() != QDialog::Accepted) return;

        // ---- editando ----
        if (editando) {
            existente->setCapacidade(cap->value());
            if (auto* t = dynamic_cast<SalaTeorica*>(existente)) {
                t->setTemProjetor(proj->isChecked());
            } else if (auto* l = dynamic_cast<Laboratorio*>(existente)) {
                l->setTipoLab(tipoLab->currentText().toStdString());
                l->setQtdComputadores(pcs->value());
            }
            persistir();
            atualizarTudo();
            statusBar()->showMessage("Sala " + QString::fromStdString(existente->getCodigo()) +
                                         " atualizada.", 5000);
            return;
        }

        // ---- adicionando ----
        QString codigo = cod->text().trimmed();
        if (codigo.isEmpty()) {
            QMessageBox::warning(this, "Atenção", "Digite o código da sala.");
            return;
        }
        std::string chave = codigo.toStdString();
        if (repo.salas.count(chave)) {
            QMessageBox::warning(this, "Atenção", "A sala " + codigo + " já existe no sistema!");
            return;
        }

        if (tipo->currentIndex() == 0) {
            repo.salas[chave] = new SalaTeorica(chave, cap->value(), proj->isChecked());
        } else {
            repo.salas[chave] = new Laboratorio(chave, cap->value(),
                                                tipoLab->currentText().toStdString(),
                                                pcs->value());
        }
        persistir();
        atualizarTudo();
        statusBar()->showMessage("Sala " + codigo + " adicionada com sucesso!", 5000);
    }

    void removerSala() {
        int row = tabSalas->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "Remover sala", "Selecione uma sala na tabela.");
            return;
        }
        QString codigo = tabSalas->item(row, 0)->text();
        if (!confirmar("Remover sala",
                       "Remover a sala " + codigo + " e todas as reservas dela?\n"
                       "Essa ação não pode ser desfeita."))
            return;

        auto it = repo.salas.find(codigo.toStdString());
        if (it != repo.salas.end()) {
            delete it->second;
            repo.salas.erase(it);
        }
        persistir();
        atualizarTudo();
        statusBar()->showMessage("Sala " + codigo + " removida.", 5000);
    }

    // ---------------- reservas ----------------
    void reservar() {
        if (comboSala->currentIndex() < 0) {
            QMessageBox::warning(this, "Atenção", "Cadastre uma sala primeiro.");
            return;
        }
        QTime ini = hIni->time();
        QTime fim = hFim->time();
        if (fim <= ini) {
            QMessageBox::warning(this, "Horário inválido",
                                 "O horário de fim precisa ser depois do início.");
            return;
        }
        if (data->date() < QDate::currentDate() &&
            !confirmar("Data no passado", "O dia escolhido já passou. Reservar mesmo assim?"))
            return;

        QString codigo = comboSala->currentText();
        std::string dia = data->date().toString("dd/MM/yyyy").toStdString();
        std::string hi = ini.toString("HH:mm").toStdString();
        std::string hf = fim.toString("HH:mm").toStdString();

        Sala* sala = repo.salas.at(codigo.toStdString());
        if (sala->verificarConflito(dia, hi, hf)) {
            QMessageBox::critical(
                this, "Conflito de horário",
                QString("A sala %1 já possui reserva no dia %2 entre %3 e %4.\n\n"
                        "Escolha outro dia/horário ou use \"Ver salas livres\".")
                    .arg(codigo, QString::fromStdString(dia),
                         QString::fromStdString(hi), QString::fromStdString(hf)));
            return;
        }

        sala->adicionarReserva(Reserva(dia, hi, hf));
        persistir();
        atualizarSalas();
        atualizarResumo();
        atualizarReservas();
        statusBar()->showMessage(
            QString("Reserva realizada: sala %1, %2 (%3 às %4).")
                .arg(codigo, QString::fromStdString(dia),
                     QString::fromStdString(hi), QString::fromStdString(hf)),
            6000);
    }

    void cancelarReserva() {
        int row = tabRes->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "Cancelar reserva",
                                     "Selecione uma reserva na tabela.");
            return;
        }
        QString cod = tabRes->item(row, 0)->text();
        int idx = tabRes->item(row, 0)->data(Qt::UserRole).toInt();
        QString desc = QString("sala %1, dia %2, das %3 às %4")
                           .arg(cod, tabRes->item(row, 1)->text(),
                                tabRes->item(row, 2)->text(), tabRes->item(row, 3)->text());

        if (!confirmar("Cancelar reserva", "Cancelar a reserva da " + desc + "?")) return;

        auto it = repo.salas.find(cod.toStdString());
        if (it == repo.salas.end()) return;

        Sala* nova = clonarSemReserva(it->second, idx);
        if (!nova) return;
        delete it->second;
        it->second = nova;

        persistir();
        atualizarSalas();
        atualizarResumo();
        atualizarReservas();
        statusBar()->showMessage("Reserva cancelada.", 5000);
    }

    // Lista as salas livres no dia/horario escolhidos
    void salasLivres() {
        QTime ini = hIni->time();
        QTime fim = hFim->time();
        if (fim <= ini) {
            QMessageBox::warning(this, "Horário inválido",
                                 "O horário de fim precisa ser depois do início.");
            return;
        }
        std::string dia = data->date().toString("dd/MM/yyyy").toStdString();
        std::string hi = ini.toString("HH:mm").toStdString();
        std::string hf = fim.toString("HH:mm").toStdString();

        QDialog d(this);
        d.setWindowTitle("Salas livres");
        d.setMinimumSize(460, 460);

        auto* info = new QLabel(QString("Salas livres em <b>%1</b>, das <b>%2</b> às <b>%3</b>:")
                                    .arg(QString::fromStdString(dia),
                                         QString::fromStdString(hi),
                                         QString::fromStdString(hf)));
        auto* minCap = new QSpinBox;
        minCap->setRange(0, 500);
        auto* linhaCap = new QHBoxLayout;
        linhaCap->addWidget(new QLabel("Capacidade mínima:"));
        linhaCap->addWidget(minCap);
        linhaCap->addStretch(1);

        auto* lista = new QListWidget;
        auto* btnUsar = new QPushButton("Usar sala selecionada");
        btnUsar->setObjectName("primario");
        auto* btnFechar = new QPushButton("Fechar");
        auto* botoes = new QHBoxLayout;
        botoes->addStretch(1);
        botoes->addWidget(btnFechar);
        botoes->addWidget(btnUsar);

        auto* lay = new QVBoxLayout(&d);
        lay->addWidget(info);
        lay->addLayout(linhaCap);
        lay->addWidget(lista, 1);
        lay->addLayout(botoes);

        auto preencher = [&] {
            lista->clear();
            for (const auto& par : repo.salas) {
                const Sala* s = par.second;
                if (s->getCapacidade() < minCap->value()) continue;
                if (s->verificarConflito(dia, hi, hf)) continue;

                QString tipo, det;
                descrever(s, tipo, det);
                auto* item = new QListWidgetItem(
                    QString("%1   ·   %2   ·   %3 lugares   ·   %4")
                        .arg(QString::fromStdString(par.first), tipo)
                        .arg(s->getCapacidade())
                        .arg(det));
                item->setData(Qt::UserRole, QString::fromStdString(par.first));
                lista->addItem(item);
            }
            if (lista->count() == 0) {
                auto* vazio = new QListWidgetItem("Nenhuma sala livre nesse horário.");
                vazio->setFlags(Qt::NoItemFlags);
                lista->addItem(vazio);
            }
        };
        preencher();
        connect(minCap, &QSpinBox::valueChanged, &d, preencher);

        QString escolhida;
        auto usar = [&] {
            auto* it = lista->currentItem();
            if (!it) return;
            QString c = it->data(Qt::UserRole).toString();
            if (c.isEmpty()) return;
            escolhida = c;
            d.accept();
        };
        connect(btnUsar, &QPushButton::clicked, &d, usar);
        connect(lista, &QListWidget::itemDoubleClicked, &d, usar);
        connect(btnFechar, &QPushButton::clicked, &d, &QDialog::reject);

        d.exec();

        if (!escolhida.isEmpty()) {
            int i = comboSala->findText(escolhida);
            if (i >= 0) comboSala->setCurrentIndex(i);
            statusBar()->showMessage("Sala " + escolhida + " selecionada. Clique em Reservar.", 6000);
        }
    }

    void persistir() {
        if (!repo.salvar())
            QMessageBox::warning(this, "Erro", "Não foi possível salvar os arquivos CSV.");
    }
};

// ------------------------------------------------------------
//  Descobre onde estao salas.csv / reservas.csv
// ------------------------------------------------------------
static QString acharPasta() {
    QDir d(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; i++) {
        if (QFile::exists(d.filePath("salas.csv")) || QFile::exists(d.filePath("reservas.csv")))
            return d.absolutePath();
        if (!d.cdUp()) break;
    }
    return QString();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    QLocale::setDefault(QLocale(QLocale::Portuguese, QLocale::Brazil));

    // Paleta fixa (vermelho do cin):
    QPalette p;
    p.setColor(QPalette::Window, QColor("#f4f4f4"));
    p.setColor(QPalette::WindowText, QColor("#2b2b2b"));
    p.setColor(QPalette::Base, QColor("#ffffff"));
    p.setColor(QPalette::AlternateBase, QColor("#fafafa"));
    p.setColor(QPalette::Text, QColor("#2b2b2b"));
    p.setColor(QPalette::PlaceholderText, QColor("#8a8a8a"));
    p.setColor(QPalette::Button, QColor("#ffffff"));
    p.setColor(QPalette::ButtonText, QColor("#2b2b2b"));
    p.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    p.setColor(QPalette::ToolTipText, QColor("#2b2b2b"));
    p.setColor(QPalette::Highlight, QColor("#D6002B"));
    p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(p);
    app.setStyleSheet(ESTILO);

    QString pasta = acharPasta();
    if (pasta.isEmpty()) {
        QMessageBox::information(nullptr, "Pasta do projeto",
                                 "Não encontrei o salas.csv automaticamente.\n"
                                 "Escolha a pasta do projeto (onde ficam salas.csv e reservas.csv).");
        pasta = QFileDialog::getExistingDirectory(nullptr, "Escolha a pasta do projeto");
        if (pasta.isEmpty()) return 0;
    }

    Janela janela(pasta);
    janela.show();
    return app.exec();
}