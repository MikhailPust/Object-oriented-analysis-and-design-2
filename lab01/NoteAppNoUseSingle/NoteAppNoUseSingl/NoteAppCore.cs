using System;
using System.Collections.Generic;
using System.Linq;

namespace NoteApp
{
    
    public class Note
    {
        public int Id { get; set; }
        public string Text { get; set; }
        public string CreateTime { get; set; }

        public Note(int id, string text)
        {
            Id = id;
            Text = text;
            CreateTime = DateTime.Now.ToString("HH:mm:ss");
        }

        public override string ToString()
        {
            return $"[{Id}] {Text}";
        }
    }

    public class Logger
    {
        private List<string> logs = new List<string>();
        public Logger() { }

        public void Log(string level, string message)
        {
            string entry = $"[{DateTime.Now:HH:mm:ss}] {level} — {message}";
            logs.Add(entry);
        }

        public List<string> GetLogs()
        {
            return logs;
        }
    }

    public class NoteRepository
    {
        private List<Note> notes = new List<Note>();
        private int nextId = 1;

        public Logger logger = new Logger();

        public void AddNote(string text)
        {
            Note note = new Note(nextId++, text);
            notes.Add(note);
            logger.Log("INFO", $"Заметка \"{text}\" создана");
        }

        public void DeleteNote(int id)
        {
            Note note = notes.Find(n => n.Id == id);
            if (note != null)
            {
                notes.Remove(note);
                logger.Log("INFO", $"Заметка \"{note.Text}\" удалена");
            }
            else
            {
                logger.Log("ERROR", $"Заметка с Id={id} не найдена");
            }
        }

        public List<Note> GetAllNotes()
        {
            return notes;
        }
    }

    public class NoteEditor
    {
        public Logger logger = new Logger();

        public void EditNote(Note note, string newText)
        {
            string oldText = note.Text;
            note.Text = newText;
            logger.Log("INFO", $"Заметка \"{oldText}\" изменена на \"{newText}\"");
        }
    }

    public class NoteListController
    {
        private NoteRepository repository;

        public Logger logger = new Logger();

        public NoteListController(NoteRepository repository)
        {
            this.repository = repository;
        }

        public List<Note> SearchNotes(string query)
        {
            logger.Log("INFO", $"Поиск по запросу: \"{query}\"");
            return repository.GetAllNotes()
                .Where(n => n.Text.ToLower().Contains(query.ToLower()))
                .ToList();
        }

        public List<Note> SortNotes()
        {
            logger.Log("INFO", "Заметки отсортированы");
            return repository.GetAllNotes()
                .OrderBy(n => n.Id)
                .ToList();
        }
    }
}
