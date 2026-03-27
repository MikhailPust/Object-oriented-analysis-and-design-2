#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QImage>
#include <QColor>

class TileCell {
private:
    int     m_x;
    int     m_y;
    bool    m_placed;
    QString m_name;
    QColor  m_color;
    QColor  m_accent;
    QImage  m_image;      
    bool    m_hasTexture;
    bool    m_hasAccent;

public:
    TileCell() : m_x(0), m_y(0), m_placed(false), m_hasTexture(false), m_hasAccent(false) {}
    TileCell(int x, int y) : m_x(x), m_y(y), m_placed(false), m_hasTexture(false), m_hasAccent(false) {}

    void render(QPainter& p, int size) const {
        if (!m_placed) {
            p.fillRect(m_x * size, m_y * size, size, size, QColor(25, 25, 35));
            return;
        }
        if (m_hasTexture && !m_image.isNull()) {
            // рисуем из QImage
            p.drawImage(QRect(m_x * size, m_y * size, size, size), m_image);
        } else if (m_hasAccent) {
            int half = size / 2;
            p.fillRect(m_x*size,        m_y*size,        half, half, m_color);
            p.fillRect(m_x*size + half, m_y*size,        half, half, m_accent);
            p.fillRect(m_x*size,        m_y*size + half, half, half, m_accent);
            p.fillRect(m_x*size + half, m_y*size + half, half, half, m_color);
        } else {
            p.fillRect(m_x * size, m_y * size, size, size, m_color);
        }
        p.setPen(QPen(QColor(0, 0, 0, 40), 0.5));
        p.drawRect(m_x * size, m_y * size, size, size);
    }

    size_t memoryBytes() const {
        if (!m_placed) return 0;
        size_t mem = sizeof(TileCell);
        mem += (size_t)m_name.size() * 2;
        if (!m_image.isNull())
            mem += (size_t)m_image.sizeInBytes(); 
        return mem;
    }

    void setTile(const QString& name, const QColor& color,
                 const QString& path, const QColor& accent,
                 bool hasTexture, bool hasAccent)
    {
        m_placed     = true;
        m_name       = name;
        m_color      = color;
        m_accent     = accent;
        m_hasTexture = hasTexture;
        m_hasAccent  = hasAccent;
        if (hasTexture)
            m_image = QImage(path).copy();
    }

    void clear() {
        m_placed     = false;
        m_name       = {};
        m_image      = {};
        m_hasTexture = false;
        m_hasAccent  = false;
    }

    bool isEmpty() const { return !m_placed; }
};

class GameMap {
private:
    int m_w, m_h;
    QVector<QVector<TileCell>> m_grid;
    bool inBounds(int x, int y) const { return x >= 0 && x < m_w && y >= 0 && y < m_h; }

public:
    GameMap(int w, int h) : m_w(w), m_h(h) {
        m_grid.resize(h);
        for (int y = 0; y < h; ++y) {
            m_grid[y].resize(w);
            for (int x = 0; x < w; ++x)
                m_grid[y][x] = TileCell(x, y);
        }
    }

    void setTile(int x, int y, const QString& name) {
        if (!inBounds(x, y)) return;
        TileCell& c = m_grid[y][x];
        if      (name == "Трава")  c.setTile("Трава",  QColor(96,  153, 72),  "textures/grass.png", {},                    true,  false);
        else if (name == "Песок")  c.setTile("Песок",  QColor(210, 178, 88),  "textures/sand.png",  {},                    true,  false);
        else if (name == "Дерево") c.setTile("Дерево", QColor(46,  100, 40),  "textures/tree.png",  {},                    true,  false);
        else if (name == "Камень") c.setTile("Камень", QColor(108, 108, 108), "textures/rock.png",  {},                    true,  false);
        else if (name == "Вода")   c.setTile("Вода",   QColor(33,  117, 176), {},                   QColor(50,  140, 200), false, true);
        else if (name == "Снег")   c.setTile("Снег",   QColor(210, 225, 240), {},                   QColor(190, 210, 230), false, true);
    }

    void clearTile(int x, int y) { if (inBounds(x,y)) m_grid[y][x].clear(); }
    void clearAll() {
        for (auto& row : m_grid)
            for (auto& cell : row)
                cell.clear();
    }

    TileCell* cell(int x, int y) { return inBounds(x,y) ? &m_grid[y][x] : nullptr; }
    int w() const { return m_w; }
    int h() const { return m_h; }

    int filledCount() const {
        int n = 0;
        for (auto& row : m_grid)
            for (auto& c : row)
                if (!c.isEmpty()) n++;
        return n;
    }

    size_t placedMemoryBytes() const {
        size_t total = 0;
        for (auto& row : m_grid)
            for (auto& c : row)
                total += c.memoryBytes();
        return total;
    }
};

static QString formatBytes(size_t bytes) {
    if (bytes < 1024)
        return QString("%1 Б").arg(bytes);
    else if (bytes < 1024 * 1024)
        return QString("%1 КБ").arg(bytes / 1024.0, 0, 'f', 1);
    else
        return QString("%1 МБ").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
}

class MapWidget : public QWidget {
    Q_OBJECT

private:
    GameMap m_map;
    int     m_cellSize;
    QString m_selected;

    void handleClick(QPoint pos, Qt::MouseButton btn) {
        int x = pos.x() / m_cellSize;
        int y = pos.y() / m_cellSize;
        if (btn == Qt::RightButton) m_map.clearTile(x, y);
        else                        m_map.setTile(x, y, m_selected);
        emit statsChanged();
        update();
    }

public:
    explicit MapWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , m_map(36, 24)
        , m_cellSize(32)
        , m_selected("Трава")
    {
        setFixedSize(m_map.w() * m_cellSize, m_map.h() * m_cellSize);
        setMouseTracking(true);
    }

    void setSelected(const QString& name) { m_selected = name; }
    GameMap& map() { return m_map; }

signals:
    void statsChanged();

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.fillRect(rect(), QColor(25, 25, 35));
        for (int y = 0; y < m_map.h(); ++y)
            for (int x = 0; x < m_map.w(); ++x)
                if (auto* c = m_map.cell(x, y))
                    c->render(p, m_cellSize);
    }

    void mousePressEvent(QMouseEvent* e) override { handleClick(e->pos(), e->button()); }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton)  handleClick(e->pos(), Qt::LeftButton);
        if (e->buttons() & Qt::RightButton) handleClick(e->pos(), Qt::RightButton);
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    MapWidget* m_mapWidget;
    QLabel*    m_statsLabel;

    QList<QPair<QString, QColor>> m_palette = {
                                               { "Трава",  QColor(96,  153, 72)  },
                                               { "Песок",  QColor(210, 178, 88)  },
                                               { "Дерево", QColor(46,  100, 40)  },
                                               { "Камень", QColor(108, 108, 108) },
                                               { "Вода",   QColor(33,  117, 176) },
                                               { "Снег",   QColor(210, 225, 240) },
                                               };

public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Редактор карты — БЕЗ паттерна Flyweight");

        QWidget* central = new QWidget(this);
        QHBoxLayout* layout = new QHBoxLayout(central);
        layout->setSpacing(8);
        layout->setContentsMargins(8, 8, 8, 8);

        m_mapWidget = new MapWidget(this);
        connect(m_mapWidget, &MapWidget::statsChanged, this, &MainWindow::updateStats);

        QWidget* sidebar = new QWidget(this);
        QVBoxLayout* sl = new QVBoxLayout(sidebar);
        sidebar->setFixedWidth(160);
        sl->setSpacing(8);

        QGroupBox* paletteBox = new QGroupBox("Тип тайла", sidebar);
        QVBoxLayout* pl = new QVBoxLayout(paletteBox);
        QButtonGroup* group = new QButtonGroup(paletteBox);
        group->setExclusive(true);

        bool first = true;
        for (const auto& [name, color] : m_palette) {
            QPushButton* btn = new QPushButton(name, paletteBox);
            btn->setCheckable(true);
            btn->setFixedHeight(34);
            if (first) { btn->setChecked(true); first = false; }
            QColor c = color;
            btn->setStyleSheet(QString(
                                   "QPushButton{background:%1;color:white;border-radius:4px;padding:4px;font-weight:bold;}"
                                   "QPushButton:checked{border:2px solid white;}"
                                   "QPushButton:hover{background:%2;}"
                                   ).arg(c.name()).arg(c.lighter(120).name()));
            connect(btn, &QPushButton::clicked, this, [this, name]() {
                m_mapWidget->setSelected(name);
            });
            group->addButton(btn);
            pl->addWidget(btn);
        }
        sl->addWidget(paletteBox);

        QPushButton* btnClear = new QPushButton("Очистить", sidebar);
        btnClear->setStyleSheet(
            "QPushButton{padding:7px;border-radius:4px;background:#B71C1C;color:white;font-weight:bold;}"
            );
        connect(btnClear, &QPushButton::clicked, this, [this]() {
            m_mapWidget->map().clearAll();
            updateStats();
            m_mapWidget->update();
        });
        sl->addWidget(btnClear);

        QGroupBox* statsBox = new QGroupBox("Статистика", sidebar);
        QVBoxLayout* stl = new QVBoxLayout(statsBox);
        m_statsLabel = new QLabel(statsBox);
        m_statsLabel->setWordWrap(true);
        m_statsLabel->setStyleSheet("font-size:10px;");
        stl->addWidget(m_statsLabel);
        sl->addWidget(statsBox);

        sl->addWidget(new QLabel("<small><b>ЛКМ</b> — разместить<br><b>ПКМ</b> — удалить</small>", sidebar));
        sl->addStretch();

        layout->addWidget(m_mapWidget);
        layout->addWidget(sidebar);
        setCentralWidget(central);

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::updateStats);
        timer->start(200);

        updateStats();
        adjustSize();
    }

private slots:
    void updateStats() {
        int    filled = m_mapWidget->map().filledCount();
        size_t mem    = m_mapWidget->map().placedMemoryBytes();

        m_statsLabel->setText(QString(
                                  "Тайлов размещено:\n  %1\n\n"
                                  "Память тайлов:\n  %2"
                                  ).arg(filled).arg(formatBytes(mem)));
    }
};

#include "main.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(30, 30, 40));
    dark.setColor(QPalette::WindowText,      Qt::white);
    dark.setColor(QPalette::Base,            QColor(20, 20, 30));
    dark.setColor(QPalette::AlternateBase,   QColor(40, 40, 55));
    dark.setColor(QPalette::Text,            Qt::white);
    dark.setColor(QPalette::Button,          QColor(45, 45, 60));
    dark.setColor(QPalette::ButtonText,      Qt::white);
    dark.setColor(QPalette::Highlight,       QColor(80, 80, 180));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(dark);

    MainWindow window;
    window.show();
    return app.exec();
}
