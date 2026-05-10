using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using irrigation_system.Data;
using irrigation_system.Models;

namespace irrigation_system.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class SensorController : ControllerBase
    {
        private readonly AppDbContext _context;

        public SensorController(AppDbContext context)
        {
            _context = context;
        }

        [HttpPost]
        public async Task<IActionResult> Post([FromBody] SensorData data)
        {
            _context.SensorData.Add(data);
            await _context.SaveChangesAsync();
            return Ok("Data saved!");
        }

        [HttpGet]
        public async Task<IActionResult> Get()
        {
            var data = await _context.SensorData.ToListAsync();
            return Ok(data);
        }
    }
}