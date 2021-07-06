<?php
    // db variables
    $host = "localhost";
    $dbname = "esp8266datos";
    $user = "root";
    $password = "";

    try{
        $conn = new PDO("mysql:host=$host;dbname=$dbname", $user, $password);
        echo '<br>Conexión exitosa a la base de datos <br><br><br>';
        
    }
    catch(Exception $e){ //PDO Exception obj 
        die('Error: ' . $e->GetMessage());  // kill the process
    }
    
    //$conn = null; // at the end we free memory and resources

?>