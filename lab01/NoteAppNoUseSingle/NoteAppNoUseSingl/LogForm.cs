using System.Collections.Generic;
using System.Windows.Forms;
using System.Drawing;

namespace NoteApp
{
    
    public class LogForm : Form
    {
        private Logger repositoryLogger;
        private Logger editorLogger;
        private Logger controllerLogger;

        public LogForm(Logger repositoryLogger, Logger editorLogger, Logger controllerLogger)
        {
            this.repositoryLogger = repositoryLogger;
            this.editorLogger = editorLogger;
            this.controllerLogger = controllerLogger;

            Text = "Журнал событий (БЕЗ паттерна — 3 разных объекта Logger)";
            Width = 620;
            Height = 500;
            StartPosition = FormStartPosition.CenterParent;

            LoadLogs();
        }

        private void LoadLogs()
        {
            
            var panel = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                ColumnCount = 1,
                RowCount = 6
            };

            // Logger #1 — NoteRepository
            panel.Controls.Add(new Label
            {
                Text = "Logger #1 — NoteRepository ",
                Font = new Font("Arial", 9, FontStyle.Bold),
                ForeColor = Color.DarkBlue,
                Height = 20,
                Dock = DockStyle.Fill
            });

            var list1 = new ListBox { Dock = DockStyle.Fill, Height = 80, Font = new Font("Courier New", 8) };
            foreach (var log in repositoryLogger.GetLogs())
                list1.Items.Add(log);
            if (list1.Items.Count == 0) list1.Items.Add("(пусто)");
            panel.Controls.Add(list1);

            // Logger #2 — NoteEditor
            panel.Controls.Add(new Label
            {
                Text = "Logger #2 — NoteEditor ",
                Font = new Font("Arial", 9, FontStyle.Bold),
                ForeColor = Color.DarkGreen,
                Height = 20,
                Dock = DockStyle.Fill
            });

            var list2 = new ListBox { Dock = DockStyle.Fill, Height = 80, Font = new Font("Courier New", 8) };
            foreach (var log in editorLogger.GetLogs())
                list2.Items.Add(log);
            if (list2.Items.Count == 0) list2.Items.Add("(пусто)");
            panel.Controls.Add(list2);

            // Logger #3 — NoteListController
            panel.Controls.Add(new Label
            {
                Text = "Logger #3 — NoteListController ",
                Font = new Font("Arial", 9, FontStyle.Bold),
                ForeColor = Color.DarkRed,
                Height = 20,
                Dock = DockStyle.Fill
            });

            var list3 = new ListBox { Dock = DockStyle.Fill, Height = 80, Font = new Font("Courier New", 8) };
            foreach (var log in controllerLogger.GetLogs())
                list3.Items.Add(log);
            if (list3.Items.Count == 0) list3.Items.Add("(пусто)");
            panel.Controls.Add(list3);

            Controls.Add(panel);
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
           
            this.ClientSize = new System.Drawing.Size(282, 253);
            this.Name = "LogForm";
            this.Load += new System.EventHandler(this.LogForm_Load);
            this.ResumeLayout(false);

        }

        private void LogForm_Load(object sender, System.EventArgs e)
        {

        }
    }
}
