lon = []
lat = []
time = []
alt = []

input = []

file = "DATALOG.TXT"

with open(file, "r") as f:
    tmp = 0
    for line in f:
        input.append(line)


v_alt = []

for i in range(len(input)):
    data = input[i].split(",")
    print(data)
    lon.append(float(data[4]))
    lat.append(float(data[5]))
    time.append(float(data[1].split(":")[0]) + float(data[1].split(":")[1]) / 60 + float(data[1].split(":")[2]) / 3600)
    #print(lon, lat, alt)
    v_alt.append(float(data[9]))
    alt.append(float(data[6]))


g = open("spaceballoon_map.html", "w")

with open('marker-popups-spaceballoon.html', "r") as f:
    for line in f:
        if line.find("//XXX") > -1:
            #print(line)
            for i in range(int(len(lon))):
                g.write("var feature = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point( "+ str(lat[i])+ ", "+ str(lon[i]) +" ).transform(epsg4326, projectTo),{description:'This is the value of<br>the description attribute'} ,{externalGraphic: 'img/marker.png', graphicHeight: 25, graphicWidth: 21, graphicXOffset:-12, graphicYOffset:-25  });vectorLayer.addFeatures(feature);")
        else:
            g.write(line)
g.close()