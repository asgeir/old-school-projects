import xml.etree.ElementTree as ET


def hungry(xml, searchLat, searchLon):
    root = ET.parse(xml).getroot()
    if root.tag != 'osm':
        return []

    closestDistance = 1e100
    closestName = None
    for amenityType in ['restaurant', 'fast_food']:
        for node in root.findall("./node/tag[@k='amenity'][@v='{0}']/..".format(amenityType)):
            name = None
            lat = None
            lon = None

            try:
                lat = float(node.get('lat'))
                lon = float(node.get('lon'))
                name = node.find("./tag[@k='name']").get('v')
            except AttributeError:
                continue

            distance = (searchLat-lat)**2 + (searchLon-lon)**2
            print('{0} - {1}'.format(name, distance))
            if distance < closestDistance:
                closestDistance = distance
                closestName = name

    return closestName


if __name__ == '__main__':
    # print(hungry('rvk.osm', 64.135738, -21.905887))
    # print(hungry('rvk.osm', 64.1433, -21.867785))
    print(hungry('rvk.osm', 64.164367, -22.021844))
