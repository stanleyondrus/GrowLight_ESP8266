var DEBUG = 0;

(function($) {
  "use strict"; // Start of use strict
  // Closes responsive menu when a scroll trigger link is clicked
  $('.js-scroll-trigger').click(function() {
    $('.navbar-collapse').collapse('hide');
  });

  // Activate scrollspy to add active class to navbar items on scroll
  $('body').scrollspy({
    target: '#mainNav',
    offset: 56
  });
})(jQuery); // End of use strict

var connection;
if (DEBUG==1) connection = new WebSocket("wss://echo.websocket.org");
else connection = new WebSocket('ws://' + location.hostname + ':81', ['arduino']);

connection.onopen = function () {
  connection.send('Connect ' + new Date());
  connection.send("~");
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  if (DEBUG==1) console.log('Server: ', e.data);
  processData(e.data)
};
connection.onclose = function(){
  console.log('WebSocket connection closed');
};

var relayState = [0,0,0,0];

var btnContainer = document.getElementById("section-settings");
var btns = btnContainer.getElementsByClassName("btn_relay");
for (var i = 0; i < btns.length; i++) {
  btns[i].addEventListener("click", function() {
    connection.send("R" + this.id + (relayState[this.id-1]^1));
  });
}

var btnModeContainer = document.getElementById("section-mode-select");
var btnsMode = btnModeContainer.getElementsByClassName("btn_mode_select");
for (var i = 0; i < btnsMode.length; i++) {
  btnsMode[i].addEventListener("click", function() {
    connection.send(this.id);
  });
}
var relaySwitches = document.getElementsByClassName("timer-relay-switch");
for (var i = 0; i < relaySwitches.length; i++) {
  relaySwitches[i].addEventListener("change", function() {
var state = this.checked?"1":"0";
connection.send(this.id + state);
  });
}

function initTimePicker(startTime, stopTime) {
  $("#timepicker").timerangewheel({
    width: 300,
    height: 300,
    indicatorWidth: 12,
    handleRadius: 15,
    handleStrokeWidth: 1,
    accentColor: '#2C3E50',
    handleIconColor: "#fff",
    handleStrokeColor: "#fff",
    handleFillColorStart: "#374149",
    handleFillColorEnd: "#374149",
    tickColor: "#8a9097",
    indicatorBackgroundColor: "#E74C3C",
    data: {"start": startTime, "end": stopTime},
    onChange: function (timeObj) {
      $(".graph-left").html(timeObj.start);
      $(".graph-right").html(timeObj.end);
      $(".graph-center").html(timeObj.duration);
      connection.send("TT" + timeObj.start + ":" + timeObj.end);
    }
  });
}

$("input[type='number']").inputSpinner();
var gmtSelect = $("#gmt_select");
gmtSelect.on("change", function (event) {
  connection.send("TO" +  gmtSelect.val());
})

function processData(data) {
  var id = data.charAt(0);

  if (id == 'M') {
    var val = parseInt(data.substring(1));
    document.getElementById("M1").classList.remove("active");
    document.getElementById("M2").classList.remove("active");
    document.getElementById('section-timer').style.display = (val == 1) ? 'none' : 'block';
    document.getElementById("M" + val).className += " active";
  }
  else if (id == 'R') {
    for (var i=0; i<4; i++) {
      relayState[i] = parseInt(data.charAt(i+1));
      document.getElementById(i+1).style.background= relayState[i] ? '#44FF44' : '#FF4444';
    }
  }
  else if (id == 'T') {
   var id = data.charAt(1);

    if (id == 'C') {
      document.getElementById("current_time").innerHTML = data.substring(2);
    }
    else if (id == 'T') {
      var s = data.substring(2).split(":",4);
      document.getElementById("start_time").innerHTML = s[0] + ":" + s[1];
      document.getElementById("end_time").innerHTML = s[2] + ":" + s[3];
      initTimePicker(s[0]+":"+s[1], s[2]+":"+s[3]);
    }
    else if (id == 'R') {
      document.getElementById("TR1").checked = data.charAt(2)==1?true:false;
      document.getElementById("TR2").checked = data.charAt(3)==1?true:false;
      document.getElementById("TR3").checked = data.charAt(4)==1?true:false;
      document.getElementById("TR4").checked = data.charAt(5)==1?true:false;
    }
    else if (id == 'O') {
      gmtSelect.val(data.substring(2));
    }
  }
}