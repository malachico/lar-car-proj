import rospy
from math import *
import sys

class MemberFunc:

    def __init__(self, defintion): #Initials a Membership Function with charictaristic array
        self.arr = defintion
    
    def isFuzzied(self, arg): #Checks if Membership Function exists, i.e. can the arg be fuzzified? or if its value is 0.
        if self.arr[0]<arg and self.arr[len(self.arr)-1]>arg:
            return True
        return False
    
    def getFuzzyValue(self, arg): #Fuzzify the value recieved............This function is used (mainly) for the input MF
        if self.isFuzzied(arg): #check if value is in bounds.
            min = self.arr[0]
            max = self.arr[-1]
            for val in self.arr:#Finds the
                if arg >= val:
                    min = val
                if arg <= val:
                    max = val
                    break
            ymin=0
            ymax=0
            if not min== self.arr[0]:
                ymin=1
            if not max == self.arr[len(self.arr)-1]:
                ymax=1
            if max==min or (max==self.arr[2] and min==self.arr[1] and len(self.arr)==4):
                return 1
            m=float((float(ymax-ymin))/float((max-min)))
            #print "m is %f max is %f min is %f"% (m, ymax, ymin)
            if m<0:
                return m*(arg-max)
            elif m==0:
                return 1
            return m*(arg-min)
        return 0
        
    def getInvertedFuzzyValue(self, arg):
        xmin=arg*(self.arr[1]-self.arr[0])+self.arr[0]
        xmax=arg*(self.arr[len(self.arr)-2]-self.arr[len(self.arr)-1])+self.arr[len(self.arr)-1]
        return [xmax, xmin]
        
    def getArea(self, value):
        lower=self.arr[len(self.arr)-1]-self.arr[0]
        [x1, x2]=self.getInvertedFuzzyValue(value)
        upper=x1-x2
        return ((upper+lower)*value/2)
        
    def getOverlapedArea(self,other, value1, value2):
        if other.arr[0]>self.arr[len(self.arr)-1] or other.arr[len(other.arr)-1]<self.arr[0]:
            return 0
        minval=value1
        if value1>value2:
            minval=value2
        smaller=other
        larger=self
        if other.arr[0]<self.arr[len(self.arr)-1]:
            smaller=self
            larger=other
        x1, x2=larger.getInvertedFuzzyValue(minval)
        y1, y2=smaller.getInvertedFuzzyValue(minval)
        m=MemberFunc([larger.arr[0], y1,x2 , smaller.arr[len(smaller.arr)-1]])
        return m.getArea(minval)

    def getCenterMass(self, arg):
        [xmax, xmin] = self.getInvertedFuzzyValue(arg)
        firstTri=float(self.arr[0])/3+(xmin*2/3)
        firstTriArea=arg*(xmin-float(self.arr[0]))/2
        rec=(xmax+xmin)/2
        recArea=(xmax-xmin)*arg
        secTri=float(self.arr[len(self.arr)-1])/3+(xmax*2/3)
        secTriArea=arg*(float(self.arr[len(self.arr)-1])-xmax)/2
        try:
            CM=(firstTri*firstTriArea+rec*recArea+secTri*secTriArea)/(firstTriArea+recArea+secTriArea)
        except:
            CM=0
        return CM
        
#--------------------Define Membership Functions Variables ----------------------------------#

OrientationMF= [ MemberFunc([-1,  -1,  -(60.0/180.0),  -(0.0/180.0)]),  MemberFunc([-(60.0/180.0),  -(30.0/180.0),  5.0/180.0]),  MemberFunc([00.0/180.0, 60.0/180.0,  1,  1]),  MemberFunc([-(5.0/180.0),  0,  (5.0/180.0)]),  MemberFunc([-5.0/180.0,  30.0/180.0,  60.0/180.0])]#[VNO NO ZO PO VPO]   

dOrientationMF= [ MemberFunc( [-180,  -180,-60,   0]),  MemberFunc( [-10,  0,  10]),  MemberFunc( [0,  60,  180, 180]),  MemberFunc( [-50,  -30,  5]),  MemberFunc( [-5,  30,  50])]#[VNO NO ZO PO VPO]   

#Distance
DistanceMF= [ MemberFunc([0, 0,  3.5, 7]),   MemberFunc([20,  30,  100000,  100000]),  MemberFunc([3.5,  13.5, 23.5])] #[CLOSE FAR VFAR] 

#Speed
SpeedMF = [ MemberFunc([0,  0.75,  1.5]) ,  MemberFunc([1,  2.5,  4]),   MemberFunc([3,  10,  17])] #[SLOW  FAST  VFAST ]


#Circular Speed
CircularSpeedMF = [MemberFunc([-0.75,  -0.3,  0]),  MemberFunc([-0.1,  0,  0.1]),  MemberFunc([0,  0.3,  0.75]),MemberFunc([-1.3,  -1.0,  -0.7]),   MemberFunc([0.7,   1.0,  1.3])] # [L  ST  R HL  HR]

#-------------------End of Definition --------------------------------------------------------------#

#---------------------Rules array -------------------------------------------------------------------#
Inp = 3 #Number of Inputs
Outp = 2 #Number of Outputs
RuleArray=[
                    [0,1,2,1,5],
                    [0,1,5,2,3],
                    [0,1,3,2,2],
                    [0,2,3,1,1],
                    [0,2,5,2,2],
                    [0,2,2,1,3],
                    [1,4,2,2,2],
                    [0,4,4,2,3],
                    [0,4,5,2,1],
                    [0,5,2,1,1],
                    [0,5,4,2,2],
                    [0,5,1,1,3],
                    [0,3,1,2,2],
                    [0,3,4,1,1],
                    [0,3,2,1,4],
                    [3,4,2,3,2],
                    [3,1,3,2,2],
                    [2,1,3,2,2],
                    [2,4,2,3,2]
                    ]
# -------------------------End of Rules -------------------------------------------------------------#
def isZero(number):
        if number:
            return 1
        else:
            return 0

def findMin(seq):
        val = 0
        min =2
        for v in seq:
            if v > val and v<min:
                min = v
        if min<2:
            return min
        return 0

def P2P(Distance, Orientation, dOrientation, factor):
        #SF=-1.0/90.0/360.0*Distance+19.0/(18.0*180)
        SF = 1/180
        if SF<1.0/360.0:
            SF=1.0/360.0
        OrientationMFflag=[0, 0, 0, 0, 0]
        dOrientationMFflag=[0, 0, 0, 0, 0]
        DistanceMFflag=[0, 0, 0]
        SpeedMFflag = [0, 0, 0]
        CircularSpeedMFflag=[0, 0, 0, 0, 0]
        
        
        if Orientation>180:
            Orientation-=360
        if Orientation<-180:
            Orientation+=360
        Orientation = Orientation*SF
        if Orientation>1:
            Orientation=1
        if Orientation<-1:
            Orientation=-1
        #print "Distance %f, Orientation %f" %(Distance, dOrientation)
        pos=0
        for i in OrientationMF: #Define which Membership functions of the Orientation are relevant
            if i.isFuzzied(Orientation):       
                OrientationMFflag[pos]=1
            pos+=1

        pos=0
        for i in dOrientationMF: #Define which Membership functions of the Orientation are relevant
            if i.isFuzzied(dOrientation):       
                dOrientationMFflag[pos]=1
            pos+=1
        pos=0  

        for i in DistanceMF: #Define which Membership functions of the Distance are relevant
            if i.isFuzzied(Distance):
                DistanceMFflag[pos]=1
            pos+=1

        
        CSpeedArea = CSpeedMoment = SpeedArea = SpeedMoment = 0
        rule=0
        for RuleCon in RuleArray:
            rule+=1
            if DistanceMF[RuleCon[0]-1].isFuzzied(Distance) or not (isZero(RuleCon[0])):
                if OrientationMF[RuleCon[1]-1].isFuzzied(Orientation) or not(isZero(RuleCon[1])):
                    if dOrientationMF[RuleCon[2]-1].isFuzzied(dOrientation) or not(isZero(RuleCon[2])):
                        #print rule, RuleCon
                        MinVal = findMin([isZero(RuleCon[0])*DistanceMF[RuleCon[0]-1].getFuzzyValue(Distance),isZero(RuleCon[1])*OrientationMF[RuleCon[1]-1].getFuzzyValue(Orientation), isZero(RuleCon[2])*dOrientationMF[RuleCon[2]-1].getFuzzyValue(dOrientation) ])

                        area=isZero(RuleCon[4])*CircularSpeedMF[RuleCon[4]-1].getArea(MinVal)*factor[RuleCon[4]-1]
                        CM=isZero(RuleCon[4])*CircularSpeedMF[RuleCon[4]-1].getCenterMass(MinVal)
                        CSpeedArea+=area
                        CSpeedMoment+=area*CM
                        
                        area=isZero(RuleCon[3])*SpeedMF[RuleCon[3]-1].getArea(MinVal)
                        CM=isZero(RuleCon[3])*SpeedMF[RuleCon[3]-1].getCenterMass(MinVal)
                        SpeedArea+=area
                        SpeedMoment+=area*CM
                        #print CSpeedArea, SpeedArea
            
        speed = 0
        CircularSpeed = 0
        if SpeedArea!=0:
            speed=SpeedMoment/SpeedArea
        if CSpeedArea!=0:
            CircularSpeed=CSpeedMoment/CSpeedArea
        #print "Moment %f Area %f" %(CSpeedMoment, CircularSpeedArea)

        return [speed, CircularSpeed]
