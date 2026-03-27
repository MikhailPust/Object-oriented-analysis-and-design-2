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
#include <QPixmap>
#include <map>

class ITileType {
public:
    virtual ~ITileType() = default;
    virtual void    draw(QPainter& p, int x, int y, int size) const = 0;
    virtual QString getName()  const = 0;
    virtual QColor  getColor() const = 0;
};

class TextureTile : public ITileType {
private:
    QString m_name;
    QColor  m_color;
    QPixmap m_pixmap;

public:
    TextureTile(const QString& name, const QString& path, const QColor& fallback)
        : m_name(name), m_color(fallback)
    {
        m_pixmap = QPixmap(path);
        if (m_pixmap.isNull())
            qWarning("Не загружена текстура: %s", qPrintable(path));
    }

    void draw(QPainter& p, int x, int y, int size) const override {
        if (!m_pixmap.isNull())
            p.drawPixmap(x, y, size, size, m_pixmap);
        else
            p.fillRect(x, y, size, size, m_color);
        p.setPen(QPen(QColor(0, 0, 0, 40), 0.5));
        p.drawRect(x, y, size, size);
    }

    QString getName()  const override { return m_name; }
    QColor  getColor() const override { return m_color; }
};

class ColorTile : public ITileType {
private:
    QString m_name;
    QColor  m_color;
    QColor  m_accent;

public:
    ColorTile(const QString& name, const QColor& color, const QColor& accent)
        : m_name(name), m_color(color), m_accent(accent) {}

    void draw(QPainter& p, int x, int y, int size) const override {
        int half = size / 2;
        p.fillRect(x, y, half, half, m_color);
        p.fillRect(x + half, y, half, half, m_accent);
        p.fillRect(x, y + half, half, half, m_accent);
        p.fillRect(x + half, y + half, half, half, m_color);
        p.setPen(QPen(QColor(0, 0, 0, 40), 0.5));
        p.drawRect(x, y, size, size);
    }

    QString getName()  const override { return m_name; }
    QColor  getColor() const override { return m_color; }
};

class TileTypeFactory {
private:
    std::map<QString, ITileType*> m_cache;
    QList<QString> m_order;

public:
    ~TileTypeFactory() {
        for (auto& pair : m_cache)
            delete pair.second;
    }

    void add(const QString& name, ITileType* tile) {
        m_order.append(name);
        m_cache[name] = tile;
    }

    ITileType* get(const QString& name) const {
        auto it = m_cache.find(name);
        return it != m_cache.end() ? it->second : nullptr;
    }

    const QList<QString>& names() const { return m_order; }
    int size() const { return (int)m_cache.size(); }
};

TileTypeFactory& tileFactory() {
    static TileTypeFactory f;
    static bool init = false;
    if (!init) {
        init = true;
        f.add("Трава",  new TextureTile("Трава",  "textures/grass.png", QColor(96,  153, 72)));
        f.add("Песок",  new TextureTile("Песок",  "textures/sand.png",  QColor(210, 178, 88)));
        f.add("Дерево", new TextureTile("Дерево", "textures/tree.png",  QColor(46,  100, 40)));
        f.add("Камень", new TextureTile("Камень", "textures/rock.png",  QColor(108, 108, 108)));
        f.add("Вода",   new ColorTile("Вода",   QColor(33,  117, 176), QColor(50,  140, 200)));
        f.add("Снег",   new ColorTile("Снег",   QColor(210, 225, 240), QColor(190, 210, 230)));
    }
    return f;
}

class TileCell {
private:
    int        m_x;
    int        m_y;
    ITileType* m_tile;

public:
    TileCell() : m_x(0), m_y(0), m_tile(nullptr) {}
    TileCell(int x, int y) : m_x(x), m_y(y), m_tile(nullptr) {}

    void render(QPainter& p, int size) const {
        if (m_tile)
            m_tile->draw(p, m_x * size, m_y * size, size);
        else
            p.fillRect(m_x * size, m_y * size, size, size, QColor(25, 25, 35));
    }

    void setTile(ITileType* t) { m_tile = t; }
    void clear()               { m_tile = nullptr; }
    bool isEmpty() const       { return m_tile == nullptr; }
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

    void setTile(int x, int y, ITileType* t) { if (inBounds(x,y)) m_grid[y][x].setTile(t); }
    void clearTile(int x, int y)             { if (inBounds(x,y)) m_grid[y][x].clear(); }
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

    // +++ только это добавлено
    size_t placedMemoryBytes() const {
        return (size_t)filledCount() * sizeof(TileCell);
    }
};

// +++ вспомогательная функция форматирования
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
    GameMap    m_map;
    int        m_cellSize;
    ITileType* m_selected;

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
        , m_selected(tileFactory().get("Трава"))
    {
        setFixedSize(m_map.w() * m_cellSize, m_map.h() * m_cellSize);
        setMouseTracking(true);
    }

    void setSelected(ITileType* t) { m_selected = t; }
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

public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Редактор карты — Flyweight Pattern");

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
        for (const QString& name : tileFactory().names()) {
            ITileType* t = tileFactory().get(name);
            QPushButton* btn = new QPushButton(name, paletteBox);
            btn->setCheckable(true);
            btn->setFixedHeight(34);
            if (first) { btn->setChecked(true); first = false; }
            QColor c = t->getColor();
            btn->setStyleSheet(QString(
                                   "QPushButton{background:%1;color:white;border-radius:4px;padding:4px;font-weight:bold;}"
                                   "QPushButton:checked{border:2px solid white;}"
                                   "QPushButton:hover{background:%2;}"
                                   ).arg(c.name()).arg(c.lighter(120).name()));
            connect(btn, &QPushButton::clicked, this, [this, name]() {
                m_mapWidget->setSelected(tileFactory().get(name));
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

        QGroupBox* statsBox = new QGroupBox("Flyweight статистика", sidebar);
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

        // +++ таймер обновляет каждые 200 мс
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::updateStats);
        timer->start(200);

        updateStats();
        adjustSize();
    }

private slots:
    void updateStats() {
        int filled = m_mapWidget->map().filledCount();
        int types  = tileFactory().size();
        // +++ считаем память только размещённых тайлов
        size_t mem = m_mapWidget->map().placedMemoryBytes();

        m_statsLabel->setText(QString(
                                  "Тайлов размещено:\n  %1\n\n"
                                  "Объектов-типов\nв кэше:\n  %2\n\n"
                                  "Память тайлов:\n  %3"
                                  ).arg(filled).arg(types).arg(formatBytes(mem)));
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