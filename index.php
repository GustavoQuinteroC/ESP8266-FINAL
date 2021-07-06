<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Sistema Incendios</title>

    <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="node_modules/bootstrap/dist/css/bootstrap.min.css">
    <style>
        body {
            background-color: #FFFFFF;
            color: black;
        }

        div.col-12 {
            /* background-color: gray; */
        }

        .myTitle {
            color: #0e153a;
            font-size: 200%;
            text-align: center;
            font-weight: bold;
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;
            margin: 10px 0;
        }

        .rule {
            height: 3px;
            width: 100%;
            background: #086972;
            margin-top: 0;
        }

        #tableModal{
            color: #0c0c0c;
        }
    </style>
</head>
<body>
    <!-- <form action="" method="POST">
        <input type="date" name="dateRequest">
        <input type="submit" name="submit" value="Search">
    </form> -->


    <!-- canvas parts grapghsics test for now -->
    <div class="myTitle">SISTEMA CONTRA INCENDIOS</div>
    <hr class="rule">
    <div class="row">
        <div class="col-12 col-lg-4">
            <canvas id="coChart" width="" height="200"></canvas>
        </div>
        <div class="col-12 col-lg-4">
            <canvas id="lpgChart" width="" height="200"></canvas>
        </div>
        <div class="col-12 col-lg-4">
            <canvas id="smokeChart" width="" height="200"></canvas>
        </div>
    </div>
    <div class="row">
        <div class="col-12 col-lg-4">
            <canvas id="tempChart" width="" height="200"></canvas>
        </div>
        <div class="col-12 col-lg-4">
            <canvas id="alarmChart" width="" height="200"></canvas>
        </div>
        <div class="col-12 col-lg-4 ">
            <div class="row col-12 d-flex justify-content-center">
            </div>
            <div class="row col-12 d-flex justify-content-center">
                <!-- Modal button -->
                
            </div>
        </div>
    </div>

    <!-- Modal -->
    <div class="modal fade" id="tableModal" tabindex="-1" role="dialog" aria-hidden="true">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <div class="modal-header">
                    <h5 class="modal-title">TABLA DE DATOS</h5>
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                </div>
                <div id="modalBody" class="modal-body d-flex justify-content-center">
                    <!-- reserved for the generated table -->
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" data-dismiss="modal">Cerrar</button>
                    <button type="button" class="btn btn-primary">Guardar</button>
                </div>
            </div>
        </div>
    </div>
    <!-- canvas parts grapghsics test for now -->



    <?php
        include('connection.php');
        
        // pick data 
        $alarmArray= array();
        $coArray = array();
        $lpgArray = array();
        $smokeArray = array();
        $tempArray = array();
        $tableHead = "<table border='1'><tr><th>alarm</th><th>co</th><th>lpg</th><th>smoke</th><th>temp</th><th>Date/Hour</th></tr>";
        $tableBody = "";

        if ($_SERVER['REQUEST_METHOD'] == 'POST'){
            $desiredDate = $_POST['dateRequest'];
            if (!empty($desiredDate)){
                $fragmented = explode("-", $desiredDate);
                $desiredDate = $fragmented[0] . "-" . $fragmented[1];
            }
           // echo "<br>".$desiredDate;
            $mysql = "SELECT * FROM estadisticas WHERE date_h LIKE '%" . $desiredDate . "%'";  //2019-05-04
        }
        else{
            $currentDate = date('Y-m');
            echo "Current Month: " . $currentDate;
            $mysql = "SELECT * FROM estadisticas WHERE date_h LIKE '%" . $currentDate . "%'";
        }

        $stmt = $conn->prepare($mysql);
        $stmt->execute();

        while ($line = $stmt->fetch(PDO::FETCH_OBJ)){
            // pushing to their arrays
            array_push($alarmArray, $line->alarm);
            array_push($coArray, $line->co);
            array_push($lpgArray, $line->lpg);
            array_push($smokeArray, $line->smoke);
            array_push($tempArray, $line->temp);

            // Appending to the table
            $tableBody .= "<tr><td>" . $line->alarm . "</td><td>" . $line->co . "</td><td>" . $line->lpg . "</td><td>" . $line->smoke."</td><td>".$line->temp."</td><td>".$line->date_h."</td></tr>";
        }
    ?>


    <!-- Optional JavaScript -->
    <!-- jQuery first, then Popper.js, then Bootstrap JS -->
    <script src="node_modules/jquery/dist/jquery.slim.js"></script>
    <script src="node_modules/popper.js/dist/popper.js"></script>
    <script src="node_modules/chart.js/dist/Chart.min.js"></script>
    <script src="node_modules/bootstrap/dist/js/bootstrap.min.js"></script>
    <script>
        const alarmData = <?php echo json_encode($alarmArray) ?>;
        const coData = <?php echo json_encode($coArray) ?>;
        const lpgData = <?php echo json_encode($lpgArray) ?>;
        const smokeData = <?php echo json_encode($smokeArray) ?>;
        const tempData = <?php echo json_encode($tempArray) ?>;
        const table = "<?php echo $tableHead.$tableBody ?>";

    //    console.log("Esta es la tabla en string: " + table);
    //    console.log(alarmData);
    //    console.log(coData);
    //    console.log(lpgData);
    //    console.log(smokeData);
    //    console.log(tempData);

        //test div
       let divTable = document.createElement("div");
        // div.style.width = "100px";
        // div.style.height = "100px";
        // div.style.background = "red";
        // div.style.color = "white";
        divTable.innerHTML = table;
        divTable.className = "text-center"; //for centere th and td
        //document.querySelector("Body").appendChild(divTable);

        // NEW modaL ELEMENT
        document.querySelector("#modalBody").appendChild(divTable);

       /* canvas and drawing parts */
       let samples = [];
       // filling samples with the correct number of elements for X axis
       let dataLength = Object.keys(alarmData).length;  // of the array Data
       for (let i = 0; i < dataLength; i++) {
           samples.push(i+1);        
       }
        
        /* CO Canvas - Graphic */
        let ctx1 = document.getElementById('coChart').getContext('2d');
        let chart1 = new Chart(ctx1, {
            // The type of chart we want to create
            type: 'line',

            // The data for our dataset
            data: {
                labels: samples,
                datasets: [{
                    label: 'MONÓXIDO DE CARBONO PPM',
                    backgroundColor: 'rgba(33, 230, 193, 0.3)',
                    borderColor: 'rgba(33, 230, 193, 1)',
                    borderWidth: 4,
                    data: coData // will be filled from database
                }],
            },

            // Configuration options go here
            options: {
                responsive: true,
                title: {
                    display: true,
                    fontColor: "#fff",
                    fontSize: 16,
                    fontFamily: 'Trebuchet MS',
                    text: 'MONÓXIDO DE CARBONO'
                },
                legend: {
                    labels: {
                        fontColor: "white",
                        fontSize: 15
                    }
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 12,
                            // stepSize: 1,
                            // beginAtZero: true
                        }
                    }],
                    xAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 11,
                        }
                    }]
                }
            }
        });

        /* LPG Canvas - Graphic */
        let ctx2 = document.getElementById('lpgChart').getContext('2d');
        let chart2 = new Chart(ctx2, {
            // The type of chart we want to create
            type: 'line',

            // The data for our dataset
            data: {
                labels: samples,
                datasets: [{
                    label: 'GAS LICUADO PPM',
                    backgroundColor: 'rgba(39, 142, 165, 0.3)',
                    borderColor: 'rgba(39, 142, 165, 1)',
                    borderWidth: 4,
                    data: lpgData // will be filled from database
                }],
            },

            // Configuration options go here
            options: {
                responsive: true,
                title: {
                    display: true,
                    fontColor: "#fff",
                    fontSize: 16,
                    fontFamily: 'Trebuchet MS',
                    text: 'GAS LICUADO DE PETRÓLEO'
                },
                legend: {
                    labels: {
                        fontColor: "white",
                        fontSize: 15
                    }
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 12,
                            // stepSize: 1,
                            // beginAtZero: true
                        }
                    }],
                    xAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 11,
                        }
                    }]
                }
            }
        });


        /* SMOKE Canvas - Graphic */
        let ctx3 = document.getElementById('smokeChart').getContext('2d');
        let chart3 = new Chart(ctx3, {
            // The type of chart we want to create
            type: 'line',

            // The data for our dataset
            data: {
                labels: samples,
                datasets: [{
                    label: 'HUMO PPM',
                    backgroundColor: 'rgba(31, 66, 135, 0.3)',
                    borderColor: 'rgba(31, 66, 135, 1)',
                    borderWidth: 4,
                    data: smokeData // will be filled from database
                }],
            },

            // Configuration options go here
            options: {
                responsive: true,
                title: {
                    display: true,
                    fontColor: "#fff",
                    fontSize: 16,
                    fontFamily: 'Trebuchet MS',
                    text: 'HUMO'
                },
                legend: {
                    labels: {
                        fontColor: "white",
                        fontSize: 15
                    }
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 12,
                            // stepSize: 1,
                            // beginAtZero: true
                        }
                    }],
                    xAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 11,
                        }
                    }]
                }
            }
        });


        /* TEMPERATURE Canvas - Graphic */
        let ctx4 = document.getElementById('tempChart').getContext('2d');
        let chart4 = new Chart(ctx4, {
            // The type of chart we want to create
            type: 'line',

            // The data for our dataset
            data: {
                labels: samples,
                datasets: [{
                    label: '  TEMPERATURA °C',
                    backgroundColor: 'rgba(7, 30, 61, 0.3)',
                    borderColor: 'rgba(7, 30, 61, 1)',
                    borderWidth: 4,
                    data: tempData // will be filled from database
                }],
            },

            // Configuration options go here
            options: {
                responsive: true,
                title: {
                    display: true,
                    fontColor: "#fff",
                    fontSize: 16,
                    fontFamily: 'Trebuchet MS',
                    text: 'TEMPERATURA'
                },
                legend: {
                    labels: {
                        fontColor: "white",
                        fontSize: 15
                    }
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 12,
                            // stepSize: 1,
                            // beginAtZero: true
                        }
                    }],
                    xAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 11,
                        }
                    }]
                }
            }
        });


        /* ALAMR Canvas - Graphic */
        let ctx5 = document.getElementById('alarmChart').getContext('2d');
        let chart5 = new Chart(ctx5, {
            // The type of chart we want to create
            type: 'line',

            // The data for our dataset
            data: {
                labels: samples,
                datasets: [{
                    label: '  ALARMAS',
                    backgroundColor: 'rgba(0, 0, 0, 0.3)',
                    borderColor: 'rgba(0, 0, 0, 1)',
                    borderWidth: 4,
                    data: alarmData // will be filled from database
                }],
            },

            // Configuration options go here
            options: {
                responsive: true,
                title: {
                    display: true,
                    fontColor: "#fff",
                    fontSize: 16,
                    fontFamily: 'Trebuchet MS',
                    text: 'ALARMAS'
                },
                legend: {
                    labels: {
                        fontColor: "white",
                        fontSize: 15
                    }
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 12,
                            // stepSize: 1,
                            // beginAtZero: true
                        }
                    }],
                    xAxes: [{
                        ticks: {
                            fontColor: "white",
                            fontSize: 11,
                        }
                    }]
                }
            }
        });
    </script>
</body>
</html>