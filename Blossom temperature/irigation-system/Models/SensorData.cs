namespace irigation_system.Models
{
    public class SensorData
    {
        public int Id { get; set; }

        public string DeviceId { get; set; } = "FRDM-MCXN947";

        public float Temperature { get; set; }

        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
    }
}