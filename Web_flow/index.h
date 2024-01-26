const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Витрата</title>
  <style type="text/css">
    body {
      background-color:#e7e7e7;
      margin: 0;
      }

    code {
      background: #c9c9c9; 
      border-radius: 6px;
      color: #000000;
      display: block;
      font: 16px/45px "Source Code Pro", Inconsolata, "Lucida Console", Terminal, "Courier New", Courier;
      padding: 10px;
      font-size: 120%;
    }
    button {
      background: #c9c9c9; 
      border-radius: 6px;
      color: #000000;
      display: block;
      padding: 10px;
      font-size: 100%;
      width: 100%;
      height: 100%;
      cursor: pointer;
    }
    button:hover {
      background-color: #ffffff
    }
    button:active {
      box-shadow: 0 5px #111;
      transform: translateY(4px);
    }
    header {
      margin: 0 5% 10px 5%;
    }
    section {
      margin: 0 1% 10px 5%;
      float: left;
      width: 60%;
    }
    aside {
      margin: 0 5% 10px 1%;
      float: right;
      width: 28%;
    }
    naside {
      margin: 0 5% 10px 5%;
      float: left;
      width: 90%;
    }
  </style>
  
  <head>
    <!-- Load plotly.js into the DOM -->
    <!-- <script src="plotly-basic.min.js"></script> -->
     <script src='https://cdn.plot.ly/plotly-2.3.1.min.js'></script> <!-- -->
  </head>
  
  <body>
    <header> 
      <code style="text-align: center; color:white; font-family: Open sans;font-weight: bold; font-size: 200%; background-color: #000000;">Відображення Данних</code>
    </header>
    <section>
      <code>Витрата води</code>
    </section>
    <aside>
      <code style="text-align: center;"><span id="water_flow">0</span> л/с</code>
    </aside>
      
    <section>
      <code>Загальна витрата</code>
    </section>
    <aside>
      <code style="text-align: center;"><span id="water_counter">0</span> м³</code>
    </aside>
    
    <section>
      <code>Час роботи</code>
    </section>
    <aside>
      <code style="text-align: center;"><span id="time_counter">0</span></code>
    </aside>
    
    <naside>
      <div id='myDiv'></div>
    </naside>

    <naside>
    </naside>
    
    <aside> 
      <button id="g_button", onclick="act_deact_graf(this)" style="text-align: center;">Зупинити графік</button>
    </aside>
    
    <naside>
    </naside>
    
    <aside> 
      <button id="g_button", onclick="document.location='/editUser'" style="text-align: center;">Змінити користувача</button>
    </aside>
    
    <script>
      let bol = 1;
      function act_deact_graf(x) {
        if(bol){
          bol = 0;
          document.getElementById("g_button").innerHTML = "Запустити графік";
          x.style.background = "#a9a9a9";
        }
        else {
          bol = 1;
          document.getElementById("g_button").innerHTML = "Зупинити графік";
          x.style.background = "#d3d3d3";
        }
      }
      
      let formattedTime = 0;
      const secondsToHms = (seconds) => {
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const remainingSeconds = seconds % 60;

        const formattedHours = String(hours).padStart(2, '0');
        const formattedMinutes = String(minutes).padStart(2, '0');
        const formattedSeconds = String(remainingSeconds).padStart(2, '0');

        return `${formattedHours}:${formattedMinutes}:${formattedSeconds}`;
      }
      
      const LitrsToM3 = (water_counter) => {
        const a_ = Math.floor(water_counter / 1000);
        const b_ = Math.floor(water_counter % 1000);

        const formatted_a = String(a_).padStart(1, '0');
        const formatted_b = String(b_).padStart(3, '0');

        return `${formatted_a}.${formatted_b}`;
      }
      
      function updateVariables() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var response = JSON.parse(this.responseText);
            document.getElementById('time_counter').innerHTML = secondsToHms(parseInt(response.variable1));
            document.getElementById('water_counter').innerHTML = LitrsToM3(parseInt(response.variable2));
            document.getElementById('water_flow').innerHTML = response.variable3;
          }
        };
        xhttp.open('GET', '/update', true);
        xhttp.send();
      }
    
      const Ax = [], Ay = [];
      setInterval( () => {
        updateVariables();
        if (bol){
          addPoint( Ax, Ay );
          if(Ay.length > 31){shiftPoint( Ax, Ay );}
            
          var cost = {
            type: "scatter",
            mode: "lines+markers",
            name: "Витрата",
            x: Ax.length, 
            y: Ay,
            line: {color: '#0058d1', shape: 'spline'}
          };
          
          var preset = {
            type: "scatter",
            mode: "markers",
            name: " ",
            x: ['0', '31'], 
            y: ['0', '25'], 
            line: {color: '#ffffff '},
          };
          
          var data = [preset, cost];
          
          var layout = {
            title: 'Графік зміни витрати',
            
             xaxis: {
              rangemode: 'tozero',
              fixedrange: true,
              title: 'Час, с',
              autotick: false,
              ticks: 'outside',
              tick0: 0,
              dtick: 1,
              ticklen: 5,
              tickwidth: 1,
              tickcolor: '#000'
            },
            
            yaxis: {
              rangemode: 'tozero',
              fixedrange: true,
              title: 'Витрата, <sup>л</sup>/<sub>с</sub>',
              autotick: false,
              ticks: 'outside',
              tick0: 0,
              dtick: 5,
              ticklen: 5,
              tickwidth: 1,
              tickcolor: '#000'
            },
            
            showlegend: false
          };
          
          Plotly.newPlot('myDiv', data, layout, {displayModeBar: false} );
        }
      }, 1000 );

      function addPoint( xs, ys) {
        xs.push( xs.length + 1 );
        ys.push( document.getElementById("water_flow").innerHTML );
      }
      
      function shiftPoint( xs, ys) {
        xs.shift();
        ys.shift();
      }
    </script>
  </body>
</html>
)=====";

const char webpage2[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <meta charset="utf-8" lang="uk" >
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Зміна даних про користувача</title>
  <style type="text/css">
    body {background-color:#e7e7e7;}

    code {
      background: #c9c9c9; 
      border-radius: 6px;
      color: #000000;
      display: block;
      font: 16px/45px "Source Code Pro", Inconsolata, "Lucida Console", Terminal, "Courier New", Courier;
      padding: 10px;
      font-size: 120%;
    }
    button {
      background: #c9c9c9; 
      border-radius: 6px;
      color: #000000;
      display: block;
      padding: 10px;
      font-size: 100%;
      width: 100%;
      height: 100%;
      cursor: pointer;
    }
    button:hover {
      background-color: #ffffff
    }
    button:active {
      box-shadow: 0 5px #111;
      transform: translateY(4px);
    }
    input[type=text] {
      background: #ffffff; 
      font-size: 100%;
      height: 100%;
      width: 100%;
      padding: 12px 20px;
      display: inline-block;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }
    header {
      margin: 0 5% 10px 5%;
    }
    section {
      margin: 0  1% 0 5%;
      float: left;
      width: 28%;
      padding: 10px;
      font-size: 120%;
    }
    naside {
      margin: 0 5% 10px 5%;
      float: left;
      width: 90%;
    }
  </style>

  <body>
    <header> 
      <code style="text-align: center;color:white;font-family: Open sans;font-weight: bold;font-size: 25px;background-color: #000000;">Зміна даних про користувача</code>
    </header>
    
    <form action='/submit' method='post'>
    
      <section>
        <label for='data'>Адреса:</label>
      </section>
      
      <naside>    
        <input lang="uk" type='text' name='adr' id='adr' placeholder="Введіть дані">
      </naside>
      
      <section>
        <label for='data2'>Номер:</label>
      </section>
      
      <naside>    
        <input lang="uk" type='text' name='num' id='num' placeholder="Введіть дані">
      </naside>
      
      <naside>
        <button type="submit" value="Змінити дані" onclick="saveD()">Змінити дані</button>
      </naside>
      
    </form>
    
    <script>
      function saveD() {
        alert("Дані збережено!");
      }
    </script>
  
  </body>
</html>
)=====";
