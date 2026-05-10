using Microsoft.EntityFrameworkCore;
using irigation_system.Models;

namespace irigation_system.Data
{
    public class AppDbContext : DbContext
    {
        public AppDbContext(DbContextOptions<AppDbContext> options) : base(options) { }

        // Aceasta va deveni tabela ta din baza de date
        public DbSet<SensorData> SensorData { get; set; }
    }
}