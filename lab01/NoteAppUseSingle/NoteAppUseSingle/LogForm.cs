using System.Windows.Forms;

namespace NoteApp
{
    public class LogForm : Form
    {
        private ListBox listBoxLogs;

        public LogForm()
        {
            Text = "Журнал событий";
            Width = 560;
            Height = 400;
            StartPosition = FormStartPosition.CenterParent;

            listBoxLogs = new ListBox
            {
                Dock = DockStyle.Fill,
                Font = new System.Drawing.Font("Courier New", 9)
            };

            Controls.Add(listBoxLogs);
            LoadLogs();
        }

        private void LoadLogs()
        {
            listBoxLogs.Items.Clear();
            foreach (string log in Logger.GetInstance().GetLogs())
                listBoxLogs.Items.Add(log);

            if (listBoxLogs.Items.Count == 0)
                listBoxLogs.Items.Add("Журнал пуст.");
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // LogForm
            // 
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
