import com.formdev.flatlaf.FlatClientProperties;
import com.formdev.flatlaf.FlatLightLaf;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.List;

public class MainForm extends JFrame {
    private CardLayout cardLayout = new CardLayout();
    private JPanel mainCardPanel = new JPanel(cardLayout);
    private JList<VacancyBackend.Vacancy> vacancyList;
    private JTextPane detailsArea;
    private JLabel statusLabel;
    private JLabel statsLabel;

    public MainForm() {
        // Установка темы
        FlatLightLaf.setup();
        UIManager.put("Button.arc", 12);
        UIManager.put("Component.arc", 12);
        UIManager.put("List.selectionArc", 12);

        setTitle("HR-Portal • Система управления вакансиями (Без паттерна)");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setSize(600, 850);
        setLocationRelativeTo(null);

        setLayout(new BorderLayout());

        // Добавляем шапку
        add(createHeader(), BorderLayout.NORTH);

        // Создаем страницы
        mainCardPanel.add(createListPage(), "LIST");
        mainCardPanel.add(createDetailsPage(), "DETAILS");

        add(mainCardPanel, BorderLayout.CENTER);

        // Добавляем статус-бар
        add(createStatusBar(), BorderLayout.SOUTH);

        // Загружаем ВСЕ данные сразу (Eager)
        loadData();
    }

    private JPanel createHeader() {
        JPanel header = new JPanel(new BorderLayout());
        header.setBackground(new Color(30, 27, 46)); // Тот самый красивый темно-синий
        header.setBorder(new EmptyBorder(15, 20, 15, 20));

        JLabel logo = new JLabel("HR-Portal");
        logo.setFont(new Font("Segoe UI", Font.BOLD, 18));
        logo.setForeground(Color.WHITE);

        JLabel modeTag = new JLabel("EAGER LOAD ACTIVE");
        modeTag.setFont(new Font("Segoe UI", Font.BOLD, 10));
        modeTag.setForeground(new Color(127, 119, 221));
        modeTag.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(127, 119, 221), 1),
                new EmptyBorder(2, 8, 2, 8)
        ));

        header.add(logo, BorderLayout.WEST);
        header.add(modeTag, BorderLayout.EAST);
        return header;
    }

    private JPanel createStatusBar() {
        JPanel bar = new JPanel(new BorderLayout());
        bar.setBackground(new Color(248, 247, 244));
        bar.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createMatteBorder(1, 0, 0, 0, new Color(224, 222, 219)),
                new EmptyBorder(8, 20, 8, 20)
        ));

        statusLabel = new JLabel("Инициализация...");
        statusLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        statusLabel.setForeground(new Color(107, 104, 128));

        statsLabel = new JLabel("—");
        statsLabel.setFont(new Font("Consolas", Font.BOLD, 12));
        statsLabel.setForeground(new Color(74, 61, 183));

        bar.add(statusLabel, BorderLayout.WEST);
        bar.add(statsLabel, BorderLayout.EAST);
        return bar;
    }

    private JPanel createListPage() {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setBackground(new Color(243, 244, 246));
        panel.setBorder(new EmptyBorder(20, 20, 20, 20));

        JLabel label = new JLabel("Актуальные предложения");
        label.setFont(new Font("Segoe UI", Font.BOLD, 22));
        label.setBorder(new EmptyBorder(0, 0, 15, 0));
        panel.add(label, BorderLayout.NORTH);

        vacancyList = new JList<>();
        vacancyList.setCellRenderer(new VacancyCellRenderer());
        vacancyList.setFixedCellHeight(100);
        vacancyList.setBackground(new Color(243, 244, 246));
        vacancyList.setBorder(null);

        vacancyList.addMouseListener(new MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                VacancyBackend.Vacancy sel = vacancyList.getSelectedValue();
                if (sel != null) showDetails(sel);
            }
        });

        JScrollPane scroll = new JScrollPane(vacancyList);
        scroll.setBorder(null);
        panel.add(scroll, BorderLayout.CENTER);
        return panel;
    }

    private JPanel createDetailsPage() {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setBackground(Color.WHITE);

        JButton backBtn = new JButton("← Назад к списку");
        backBtn.setFont(new Font("Segoe UI", Font.BOLD, 14));
        backBtn.setForeground(new Color(37, 99, 235));
        backBtn.addActionListener(e -> cardLayout.show(mainCardPanel, "LIST"));
        backBtn.setBorder(new EmptyBorder(15, 20, 10, 20));
        backBtn.setContentAreaFilled(false);
        backBtn.setCursor(new Cursor(Cursor.HAND_CURSOR));

        detailsArea = new JTextPane();
        detailsArea.setContentType("text/html");
        detailsArea.setEditable(false);

        JScrollPane scroll = new JScrollPane(detailsArea);
        scroll.setBorder(new EmptyBorder(0, 10, 10, 10));

        panel.add(backBtn, BorderLayout.NORTH);
        panel.add(scroll, BorderLayout.CENTER);
        return panel;
    }

    private void showDetails(VacancyBackend.Vacancy v) {
        long start = System.currentTimeMillis();

        // БЕЗ ПАТТЕРНА: Мы просто берем данные, которые УЖЕ загружены в объект при старте
        VacancyBackend.VacancyDetails d = v.getDetails();

        long time = System.currentTimeMillis() - start;

        // Время всегда будет 0 мс, так как запроса к БД нет
        statsLabel.setText("В памяти: " + time + " ms");
        statusLabel.setText("Данные из кэша: " + v.companyName);

        String html = "<html><body style='font-family: Segoe UI; padding: 15px;'>" +
                "<h1 style='margin-bottom:0; color:#1e1b2e;'>" + v.title + "</h1>" +
                "<div style='color:#6b7280; font-size:14px; margin-bottom:10px;'>" + v.companyName + " • " + v.city + "</div>" +
                "<div style='color:#2563eb; font-size:18px; font-weight:bold; margin-bottom:20px;'>от " + v.salaryFrom + " ₽</div>" +

                "<div style='background:#f3f4f6; padding:15px; border-radius:10px; margin-bottom:20px;'>" +
                "<table border='0' cellspacing='0' cellpadding='2'>" +
                "<tr><td><b>Опыт работы:</b></td><td style='padding-left:10px;'>" + d.experience + "</td></tr>" +
                "<tr><td><b>График:</b></td><td style='padding-left:10px;'>" + d.schedule + "</td></tr>" +
                "<tr><td><b>Условия:</b></td><td style='padding-left:10px;'>" + d.conditions + "</td></tr>" +
                "</table></div>" +

                "<h3 style='color:#1e1b2e;'>Описание вакансии</h3>" +
                "<p style='color:#374151; line-height:1.4;'>" + d.description + "</p>" +

                "<h3 style='color:#1e1b2e;'>Требования</h3>" +
                "<p style='color:#374151; line-height:1.4;'>" + d.requirements + "</p>" +
                "</body></html>";

        detailsArea.setText(html);
        detailsArea.setCaretPosition(0);
        cardLayout.show(mainCardPanel, "DETAILS");
        vacancyList.clearSelection();
    }

    private void loadData() {
        new SwingWorker<List<VacancyBackend.Vacancy>, Void>() {
            @Override protected List<VacancyBackend.Vacancy> doInBackground() {
                return VacancyBackend.Database.loadEverythingAtOnce();
            }
            @Override protected void done() {
                try {
                    DefaultListModel<VacancyBackend.Vacancy> m = new DefaultListModel<>();
                    for (VacancyBackend.Vacancy v : get()) m.addElement(v);
                    vacancyList.setModel(m);
                    statusLabel.setText("Готово");
                    statsLabel.setText("Все данные предзагружены");
                } catch (Exception e) { e.printStackTrace(); }
            }
        }.execute();
    }


    private static class VacancyCellRenderer extends JPanel implements ListCellRenderer<VacancyBackend.Vacancy> {
        private JLabel iconLabel = new JLabel();
        private JLabel titleLabel = new JLabel();
        private JLabel companyLabel = new JLabel();
        private JLabel salaryLabel = new JLabel();

        public VacancyCellRenderer() {
            setLayout(new BorderLayout(15, 0));
            setBorder(new EmptyBorder(12, 15, 12, 15));
            setOpaque(true);

            iconLabel.setPreferredSize(new Dimension(50, 50));
            iconLabel.setOpaque(true);
            iconLabel.setHorizontalAlignment(SwingConstants.CENTER);
            iconLabel.setFont(new Font("Segoe UI", Font.BOLD, 18));
            iconLabel.setForeground(Color.WHITE);

            JPanel textPanel = new JPanel();
            textPanel.setLayout(new BoxLayout(textPanel, BoxLayout.Y_AXIS));
            textPanel.setOpaque(false);

            titleLabel.setFont(new Font("Segoe UI Semibold", Font.PLAIN, 15));
            companyLabel.setFont(new Font("Segoe UI", Font.PLAIN, 13));
            salaryLabel.setFont(new Font("Segoe UI Semibold", Font.PLAIN, 13));

            textPanel.add(titleLabel);
            textPanel.add(Box.createVerticalStrut(2));
            textPanel.add(companyLabel);
            textPanel.add(Box.createVerticalStrut(4));
            textPanel.add(salaryLabel);

            add(iconLabel, BorderLayout.WEST);
            add(textPanel, BorderLayout.CENTER);
        }

        @Override
        public Component getListCellRendererComponent(JList<? extends VacancyBackend.Vacancy> list,
                                                      VacancyBackend.Vacancy v, int idx, boolean isSel, boolean cellFocus) {
            titleLabel.setText(v.title);
            companyLabel.setText(v.companyName + " • " + v.city);
            salaryLabel.setText("от " + v.salaryFrom + " ₽");

            iconLabel.setText(v.title.substring(0, 1).toUpperCase());
            iconLabel.setBackground(isSel ? Color.WHITE : new Color(37, 99, 235));
            iconLabel.setForeground(isSel ? new Color(37, 99, 235) : Color.WHITE);

            setBackground(isSel ? new Color(37, 99, 235) : Color.WHITE);
            titleLabel.setForeground(isSel ? Color.WHITE : Color.BLACK);
            companyLabel.setForeground(isSel ? new Color(200, 210, 255) : new Color(107, 114, 128));
            salaryLabel.setForeground(isSel ? Color.WHITE : new Color(37, 99, 235));

            setBorder(BorderFactory.createCompoundBorder(
                    new EmptyBorder(4, 0, 4, 0),
                    BorderFactory.createLineBorder(new Color(230, 230, 230), 1, true)
            ));

            return this;
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new MainForm().setVisible(true));
    }
}