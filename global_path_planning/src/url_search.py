import urllib

class WayPoint:
    def __init__(self,dist,dur,start,end):
        self.distance = dist
        self.duration = dur
        self.start_location = start
        self.end_location = end
    def print_point(self):
        print ' Distance:',self.distance
        print ' Duration:',self.duration
        print ' Start location:',self.start_location
        print ' end location:',self.end_location

def parse_distance(string):
    try:
        st = string.find(':')+3
        end = string.find(' ',st)
        val = float(string[st:end])
        unit = string[end+1:string.find('"',end+1)]
        if unit == 'm':
            return val
        if unit == 'km':
            return val*1000
        if unit == 'mi':
            return val*1609
        if unit == 'ft':
            return val*0.3048
        print 'Warning: Unrecognized unit value. returning 0',string
    except:
        print 'warning: Value to big. returning 0',string
        return 0

def parse_lat_lng(string):
    return [float(string[0][string[0].find(':')+2:string[0].find(',')]),
            float(string[1][string[1].find(':')+2:string[1].find(',')])]
            

def parse_duration(string):
    try:
        st = string.find(':') + 3
        end =string.find(' ',st)
        if st == 2 or end == -1:
            print 'warning: problem parsing duration.',string
            return 0
        val = int(string[st:end])
        unit = string[end+1:string.find('"',end)]
        if unit == 'mins' or unit == 'min':
            return val
        if unit == 'hour':
            return val*60
        if string.find('hours') == -1:
            string = ': "'+string[string.find('hour')+5:]
        else:
            string = ': "'+string[string.find('hours')+6:]
        return val*60 + parse_duration(string)
    except:
        print 'Warning: Could not parse duration',string
        return 0
    
def crop_data(data,field):
    idx = data.find(field)
    data = data[idx:]
    idx = data.find('\n')
    data = data[idx+1:]
    val = data[data.find('text'):data.find('\n')]
    return val

def crop_coordinates(data,field):
    idx = data.find(field)
    data = data[idx:]
    idx = data.find('\n')
    data = data[idx+1:]
    lat = data[data.find('lat'):data.find('\n')]
    idx = data.find('\n',data.find('lng'))
    lng = data[data.find('lng'):idx]
    return [lat,lng],idx

def crop_step(steps):
    lst = []
    idx = 0
    while idx != -1:
        dist = parse_distance(crop_data(steps,'distance'))
        dur = parse_duration(crop_data(steps,'duration'))
        end,idx = crop_coordinates(steps,'end_location')
        end = parse_lat_lng(end)
        st,idx = crop_coordinates(steps,'start_location')
        st = parse_lat_lng(st)
        wp = WayPoint(dist,dur,st,end)
        lst.append(wp)
        idx = steps.find('distance',idx)
        steps = steps[idx:]
    return lst

def get_directions(orig,dest):
    gurl = 'https://maps.googleapis.com/maps/api/directions/json?origin='+orig+'&destination='+dest+'&key=AIzaSyBM0qAALsDhCIM6oGTrHGdXiB6r4VHrZ0w'
    print gurl
    directions = urllib.urlopen(gurl).read()
    start_indx = directions.find('"steps"')
    end_indx = directions.find(']',start_indx)
    steps = crop_step(directions[start_indx:])
    indx = directions.find('"status"',end_indx)
    status = directions[indx:directions.find('\n',indx)]
    status = status[status.find(':')+2:]
    return steps,status

orig = 'Hazamir 12,Rishon Lezion'#'Tel aviv'#'New York'#
dest = 'Kaplan 20, Qiryat Ono'#"Beer Sheva"#'Los Angeles'
print 'getting directions from ',orig,' to', dest
direction_list,status = get_directions(orig,dest)
print 'status is: ',status
print 'The directions are: '
tot_dist = 0
tot_dur = 0
for wp in direction_list:
    wp.print_point()
    print '-------------'
    tot_dist += wp.distance
    tot_dur += wp.duration
print 'Total Disance= ',tot_dist/1000.0
print 'Total Duration= ',tot_dur/60.0
