using System;
using System.Collections.Generic;
using System.Windows.Forms;


namespace NoteApp
{
    public class MainForm : Form
    {
        private NoteRepository repository = new NoteRepository();
        private NoteEditor editor = new NoteEditor();
        private NoteListController controller;

        private ListBox listBoxNotes;
        private TextBox textBoxInput;
        private TextBox textBoxSearch;
        private Button btnAdd;
        private Button btnDelete;
        private Button btnEdit;
        private Button btnSearch;
        private Button btnSort;
        private Button btnOpenLog;

        public MainForm()
        {
            controller = new NoteListController(repository);
            InitializeComponents();
        }

        private void InitializeComponents()
        {
            Text = "Менеджер заметок";
            Width = 460;
            Height = 520;
            StartPosition = FormStartPosition.CenterScreen;

            // Поле ввода новой заметки
            textBoxInput = new TextBox
            {
                Left = 10, Top = 10, Width = 330
            };

            // Кнопка добавить
            btnAdd = new Button
            {
                Text = "Добавить",
                Left = 350, Top = 10, Width = 90
            };
            btnAdd.Click += BtnAdd_Click;

            // Список заметок
            listBoxNotes = new ListBox
            {
                Left = 10, Top = 45, Width = 430, Height = 250
            };

            // Кнопки редактировать и удалить
            btnEdit = new Button
            {
                Text = "Редактировать",
                Left = 10, Top = 305, Width = 130
            };
            btnEdit.Click += BtnEdit_Click;

            btnDelete = new Button
            {
                Text = "Удалить",
                Left = 150, Top = 305, Width = 90
            };
            btnDelete.Click += BtnDelete_Click;

            // Поиск
            textBoxSearch = new TextBox
            {
                Left = 10, Top = 350, Width = 240
            };

            btnSearch = new Button
            {
                Text = "Найти",
                Left = 260, Top = 350, Width = 80
            };
            btnSearch.Click += BtnSearch_Click;

            btnSort = new Button
            {
                Text = "Сортировать",
                Left = 350, Top = 350, Width = 90
            };
            btnSort.Click += BtnSort_Click;

            // Кнопка открыть журнал
            btnOpenLog = new Button
            {
                Text = "Открыть журнал событий",
                Left = 10, Top = 410, Width = 430, Height = 40
            };
            btnOpenLog.Click += BtnOpenLog_Click;

            Controls.AddRange(new Control[] {
                textBoxInput, btnAdd,
                listBoxNotes,
                btnEdit, btnDelete,
                textBoxSearch, btnSearch, btnSort,
                btnOpenLog
            });
        }

        private void RefreshList(List<Note> notes = null)
        {
            listBoxNotes.Items.Clear();
            var source = notes ?? repository.GetAllNotes();
            foreach (var note in source)
                listBoxNotes.Items.Add(note);
        }

        private void BtnAdd_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(textBoxInput.Text)) return;
            repository.AddNote(textBoxInput.Text.Trim());
            textBoxInput.Clear();
            RefreshList();
        }

        private void BtnDelete_Click(object sender, EventArgs e)
        {
            if (listBoxNotes.SelectedItem is Note note)
            {
                repository.DeleteNote(note.Id);
                RefreshList();
            }
            else
            {
                MessageBox.Show("Выберите заметку для удаления.");
            }
        }

        private void BtnEdit_Click(object sender, EventArgs e)
        {
            if (listBoxNotes.SelectedItem is Note note)
            {
                string newText = Microsoft.VisualBasic.Interaction.InputBox(
                    "Введите новый текст:", "Редактирование", note.Text);
                if (!string.IsNullOrWhiteSpace(newText))
                {
                    editor.EditNote(note, newText);
                    RefreshList();
                }
            }
            else
            {
                MessageBox.Show("Выберите заметку для редактирования.");
            }
        }

        private void BtnSearch_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(textBoxSearch.Text)) return;
            var results = controller.SearchNotes(textBoxSearch.Text);
            RefreshList(results);
        }

        private void BtnSort_Click(object sender, EventArgs e)
        {
            var sorted = controller.SortNotes();
            RefreshList(sorted);
        }

        private void BtnOpenLog_Click(object sender, EventArgs e)
        {
            new LogForm().ShowDialog(this);
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
          
            this.ClientSize = new System.Drawing.Size(648, 550);
            this.Name = "MainForm";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.ResumeLayout(false);

        }

        private void MainForm_Load(object sender, EventArgs e)
        {

        }
    }
}
