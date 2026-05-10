using irigation_system. Data;
using irigation_system.Models;
using Microsoft.AspNetCore.Mvc;


namespace irigation_system.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class DataController : ControllerBase
    {
        private readonly AppDbContext _context;

        public DataController(AppDbContext context)
        {
            _context = context;
        }

        // Metoda POST: apelată când placa trimite date
        // URL-ul va fi: http://localhost:<port>/api/data
        [HttpPost]
        public async Task<IActionResult> PostData([FromBody] SensorData incomingData)
        {
            if (incomingData == null)
            {
                return BadRequest("Datele nu au fost primite corect.");
            }

            // Setăm ora exactă a serverului în momentul primirii
            incomingData.Timestamp = DateTime.UtcNow;

            // Adăugăm în baza de date
            _context.SensorData.Add(incomingData);
            await _context.SaveChangesAsync();

            return Ok(new { message = "Data received successfully" });
        }
    }
}