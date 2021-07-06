<?php
    include('connection.php');

    $alarm = $_GET['alarm'];
    $co = $_GET['co'];
    $lpg = $_GET['lpg'];
    $smoke = $_GET['smoke'];
    $temp = $_GET['temp'];
    
                                                // sql values                          //php values
    $mysql = "INSERT INTO `estadisticas` (`alarm`, `co`, `lpg`, `smoke`, `temp`) VALUES (:alarm, :co, :lpg, :smoke, :temp)";
    


    // prepare sql and bind parameters
    $stmt = $conn->prepare($mysql);
    $stmt->bindParam(':alarm', $alarm);
    $stmt->bindParam(':co', $co);
    $stmt->bindParam(':lpg', $lpg);
    $stmt->bindParam(':smoke', $smoke);
    $stmt->bindParam(':temp', $temp);

    if ($stmt->execute()){
        echo "Guardado correcto!";
    }
    else{
        echo "Error al guardar";
    }
?>

