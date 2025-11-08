<?php

// just some boilerplate ai wrote to fetch data from the c backend server

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, "http://localhost:6969");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$response = curl_exec($ch);
curl_close($ch);

$data = json_decode($response, true);
if ($data && isset($data['records'])) {
    echo "<h1>Records</h1>";
    foreach ($data['records'] as $record) {
        echo "ID: " . $record['id'] . ", Name: " . $record['name'] . ", Job: " . $record['job'] . "<br>";
    }
} else {
    echo "No records found.";
}
?>