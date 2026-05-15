import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class VacancyBackend {

    public static class VacancyDetails {
        public String description, requirements, conditions, schedule, experience;

        public VacancyDetails(String d, String r, String c, String s, String e) {
            this.description = d; this.requirements = r;
            this.conditions = c; this.schedule = s; this.experience = e;
        }
    }

    public static class Vacancy {
        public int id;
        public String title, companyName, city;
        public int salaryFrom;

        public VacancyDetails details;

        public Vacancy(int id, String title, int salary, String comp, String city, VacancyDetails details) {
            this.id = id;
            this.title = title;
            this.salaryFrom = salary;
            this.companyName = comp;
            this.city = city;
            this.details = details; // Данные уже в памяти
        }

        public VacancyDetails getDetails() {

            return this.details;
        }
    }

    public static class Database {
        private static final String URL = "jdbc:mysql://localhost:3306/vacancy_catalog";
        private static final String USER = "root";
        private static final String PASS = "1234"; // Ваш пароль

        public static List<Vacancy> loadEverythingAtOnce() {
            List<Vacancy> list = new ArrayList<>();
            String sql = "SELECT v.*, e.company_name, d.description, d.requirements, d.conditions, d.schedule, d.experience " +
                    "FROM vacancies v " +
                    "JOIN employers e ON v.company_id = e.id " +
                    "JOIN vacancy_details d ON v.id = d.vacancy_id";

            try (Connection conn = DriverManager.getConnection(URL, USER, PASS);
                 Statement stmt = conn.createStatement();
                 ResultSet rs = stmt.executeQuery(sql)) {
                while (rs.next()) {
                    VacancyDetails details = new VacancyDetails(
                            rs.getString("description"), rs.getString("requirements"),
                            rs.getString("conditions"), rs.getString("schedule"), rs.getString("experience")
                    );
                    list.add(new Vacancy(
                            rs.getInt("id"), rs.getString("title"), rs.getInt("salary_from"),
                            rs.getString("company_name"), rs.getString("city"), details
                    ));
                }
            } catch (SQLException e) { e.printStackTrace(); }
            return list;
        }
    }
}