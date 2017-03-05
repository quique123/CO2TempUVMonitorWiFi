<html>
<body>
<?php
$servername = "localhost";
$username = "myuser";
$password = "mypwd";
$dbname = "test";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT * FROM someTable where id > (SELECT MAX(id) - 20 FROM someTable)";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    // output data of each row
    while($row = $result->fetch_assoc()) {
        //echo "id: " . $row["id"]. " - CO2PPM: " . $row["age"]. "--> @ --> " . $row["timestamp"]. "<br>";
        echo "id=" . $row["id"]. ". Temp=" . $row["name"]. ". CO2PPM=" . $row["age"]. ". UVIndex=" . $row["uvindex"]. ". MQ2R=" . $row["mq2"]. ". Timestamp=" . $row["timestamp"]. "<br>";
    }
} else {
    echo "0 results";
}
$conn->close();
?>
</body>
</html>
