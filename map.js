access_token = 'pk.eyJ1Ijoic25vcmZhbG9ycGFndXMiLCJhIjoic1oxRTMxcyJ9.eC6cjtLmFs9EcVwmT1isOQ';

var map = L.map('map').setView([51.505, -0.09], 13);

L.tileLayer('https://{s}.tiles.mapbox.com/v4/examples.map-i86nkdio/{z}/{x}/{y}@2x.png?access_token='+access_token, {
    maxZoom: 18,
    attribution: 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, ' +
        '<a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
        'Imagery &copy; <a href="http://mapbox.com">Mapbox</a>',
    id: 'examples.map-i86nkdio',
}).addTo(map);

var marker = L.marker(map.getCenter()).addTo(map);
marker.bindPopup("Hello World!").openPopup();

if(typeof MainWindow != 'undefined') {
    var onMapMove = function() { MainWindow.onMapMove(map.getCenter().lat, map.getCenter().lng) };
    map.on('move', onMapMove);
    onMapMove();
}
