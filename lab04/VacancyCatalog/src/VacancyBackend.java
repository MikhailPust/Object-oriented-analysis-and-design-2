import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class VacancyBackend {

    public static class VacancyDetails {
        public String description;
        public String requirements;
        public String conditions;
        public String schedule;
        public String experience;

        public VacancyDetails(String description, String requirements, String conditions, String schedule, String experience) {
            this.description = description;
            this.requirements = requirements;
            this.conditions = conditions;
            this.schedule = schedule;
            this.experience = experience;
        }
    }

    public static class Vacancy {
        public int id;
        public String title;
        public int salaryFrom;
        public String companyName;
        public String city;

        private VacancyDetails details = null;

        public Vacancy(int id, String title, int salaryFrom, String companyName, String city) {
            this.id = id;
            this.title = title;
            this.salaryFrom = salaryFrom;
            this.companyName = companyName;
            this.city = city;
        }

        public VacancyDetails getDetails() {
            if (this.details == null) {
                System.out.println("[LazyLoad] Загрузка деталей из БД для вакансии #" + this.id);
                this.details = Database.loadDetailsFromDb(this.id);
            } else {
                System.out.println("[LazyLoad] Детали уже в памяти. Отдаем кэш.");
            }
            return this.details;
        }
    }

    public static class Database {
        private static final String URL = "jdbc:mysql://localhost:3306/vacancy_catalog";
        private static final String USER = "root";
        private static final String PASS = "1234";

        public static List<Vacancy> loadAllVacancies() {
            List<Vacancy> list = new ArrayList<>();

            String query = "SELECT v.id, v.title, v.salary_from, v.city, e.company_name " +
                    "FROM vacancies v " +
                    "JOIN employers e ON v.company_id = e.id";

            try (Connection conn = DriverManager.getConnection(URL, USER, PASS);
                 Statement stmt = conn.createStatement();
                 ResultSet rs = stmt.executeQuery(query)) {

                while (rs.next()) {
                    list.add(new Vacancy(
                            rs.getInt("id"),
                            rs.getString("title"),
                            rs.getInt("salary_from"),
                            rs.getString("company_name"),
                            rs.getString("city")
                    ));
                }
            } catch (SQLException e) {
                System.err.println("Ошибка БД при загрузке списка: " + e.getMessage());
            }
            return list;
        }
        public static VacancyDetails loadDetailsFromDb(int vacancyId) {
            String query = "SELECT description, requirements, conditions, schedule, experience " +
                    "FROM vacancy_details WHERE vacancy_id = " + vacancyId;

            try (Connection conn = DriverManager.getConnection(URL, USER, PASS);
                 Statement stmt = conn.createStatement();
                 ResultSet rs = stmt.executeQuery(query)) {

                if (rs.next()) {
                    return new VacancyDetails(
                            rs.getString("description"),
                            rs.getString("requirements"),
                            rs.getString("conditions"),
                            rs.getString("schedule"),
                            rs.getString("experience")
                    );
                }
            } catch (SQLException e) {
                System.err.println("Ошибка БД при загрузке деталей: " + e.getMessage());
            }
            // Возвращаем заглушку, если для вакансии случайно нет описания в БД
            return new VacancyDetails("Нет данных", "Нет данных", "Нет данных", "Нет данных", "Нет данных");
        }
    }
}