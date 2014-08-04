class MemberFunc():
    arr=0;
    def __init__(self, defintion): #Initials a Membership Function with charictaristic array
        self.arr=defintion
    
    def isFuzzied(self, arg): #Checks if Membership Function exists, i.e. can the arg be fuzzified? or if its value is 0.
        if self.arr[0]<arg and self.arr[len(self.arr)-1]>arg:
            return True
        return False
    
    def getFuzzyValue(self, arg): #Fuzzify the value recieved............This function is used (mainly) for the input MF
        if self.isFuzzied(arg): #check if value is in bounds.
            min=self.arr[0]
            max=self.arr[len(self.arr)-1]
            for val in self.arr: #Finds the 
                if arg >= val:
                    min=val
                if arg <=val:
                    max=val
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
        #print "arg:%f Xmax:%f Xmin: %f" %(arg, xmax, xmin)
        firstTri=float(self.arr[0])/3+(xmin*2/3)
        firstTriArea=arg*(xmin-float(self.arr[0]))/2
        #print "FT: CM: %f, Area: %f" %(firstTri, firstTriArea)
        rec=(xmax+xmin)/2
        recArea=(xmax-xmin)*arg
        #print "rec: CM: %f, Area: %f" %(rec, recArea)
        secTri=float(self.arr[len(self.arr)-1])/3+(xmax*2/3)
        secTriArea=arg*(float(self.arr[len(self.arr)-1])-xmax)/2
        #print "ST: CM: %f, Area: %f" %(secTri, secTriArea)
        CM=(firstTri*firstTriArea+rec*recArea+secTri*secTriArea)/(firstTriArea+recArea+secTriArea)
        return CM
        
#--------------------Define Membership Functions Variables ----------------------------------#

OrientationMF= [ MemberFunc([-180,  -180,  -90,  -60]),  MemberFunc([-70,  -30,  -20]),  MemberFunc([-40,  0,  40]),  MemberFunc([20,  30,  70]),  MemberFunc([60,  90,  180,  180])]#[VNO NO ZO PO VPO]   

#Distance
DistanceMF= [ MemberFunc([0, 0,  1, 2]),  MemberFunc([1,  3, 5]),   MemberFunc([4,  7,  100000,  100000])] #[CLOSE FAR VFAR] 

#Speed
SpeedMF = [ MemberFunc([1,  1.5,  2]) ,  MemberFunc([1.5,  2.25,  3]),   MemberFunc([2.5,  6,  6])] #[SLOW  FAST  VFAST ]


#Circular Speed
CircularSpeedMF = [ MemberFunc([-12,  -9,  -6]),  MemberFunc([-4,  -2,  0]),  MemberFunc([-2,  0,  2]),  MemberFunc([0,  2,  4]),   MemberFunc([6,   9,  12])] # [HL L  ST  R  HR]

#-------------------End of Definition --------------------------------------------------------------#

#---------------------Rules array -------------------------------------------------------------------#
RuleArray=[
                    [0, -1, 0, 0],
                    [1, 0, 1, 0],
                    [1, 2, 1, 1], 
                    [1, 1, 1, 1],
                    [3, 1, 3, 1],
                    [3, 2, 3, 1], 
                    [2, 0, 2, 0],
                    [2, 1, 2, 1],
                    [2, 2, 2, 2],
                    [3, 0, 3, 0],
                    [4, -1, 4, 0]
                    ]
# -------------------------End of Rules -------------------------------------------------------------#

def P2P(Distance, dOrientation):
        
        OrientationMFflag=[0, 0, 0, 0, 0]
        DistanceMFflag=[0, 0, 0]
        SpeedMFflag = [0, 0, 0]
        CircularSpeedMFflag=[0, 0, 0, 0, 0]
        
        
        if dOrientation>180:
            dOrientation-=360
        if dOrientation<-180:
            dOrientation+=360
        #print "Distance %f, Orientation %f" %(Distance, dOrientation)
        pos=0
        for i in OrientationMF: #Define which Membership functions of the Orientation are relevant
            if i.isFuzzied(dOrientation):       
                OrientationMFflag[pos]=1
            pos+=1
        pos=0
        for i in DistanceMF: #Define which Membership functions of the Distance are relevant
            if i.isFuzzied(Distance):
                DistanceMFflag[pos]=1
            pos+=1
        
        
        CSpeedArea=CSpeedMoment=SpeedArea=SpeedMoment=0
        rule=-1
        for RuleCon in RuleArray:
            rule+=1
            Area=CM=0
            if RuleCon[0]==-1 and DistanceMFflag[RuleCon[1]]==1:
                Area=CircularSpeedMF[RuleCon[2]].getArea(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                CM=CircularSpeedMF[RuleCon[2]].getCenterMass(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                #print "Rule %d:Circular Speed->rules Area: %f rules CM: %f" %(rule+1, Area, CM)

                CSpeedArea+=Area
                CSpeedMoment+=Area*CM
                
                Area=SpeedMF[RuleCon[3]].getArea(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                CM=SpeedMF[RuleCon[3]].getCenterMass(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                #print "Rule %d: Linear Speed -> rules Area: %f rules CM: %f" %(rule+1, Area, CM)
                
                SpeedArea+=Area
                SpeedMoment+=Area*CM
            elif RuleCon[1]==-1 and OrientationMFflag[RuleCon[0]]==1:
                Area=CircularSpeedMF[RuleCon[2]].getArea(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                CM=CircularSpeedMF[RuleCon[2]].getCenterMass(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                #print "Rule %d:Circular Speed->rules Area: %f rules CM: %f" %(rule+1, Area, CM)

                CSpeedArea+=Area
                CSpeedMoment+=Area*CM
                
                Area=SpeedMF[RuleCon[3]].getArea(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                CM=SpeedMF[RuleCon[3]].getCenterMass(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                #print "Rule %d: Linear Speed -> rules Area: %f rules CM: %f" %(rule+1, Area, CM)
                
                SpeedArea+=Area
                SpeedMoment+=Area*CM
            elif DistanceMFflag[RuleCon[1]]==1 and OrientationMFflag[RuleCon[0]]==1:
                Area=CircularSpeedMF[RuleCon[2]].getArea(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                CM=CircularSpeedMF[RuleCon[2]].getCenterMass(OrientationMF[RuleCon[0]].getFuzzyValue(dOrientation))
                #print "Rule %d:Circular Speed->rules Area: %f rules CM: %f" %(rule+1, Area, CM)

                CSpeedArea+=Area
                CSpeedMoment+=Area*CM
                
                Area=SpeedMF[RuleCon[3]].getArea(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                CM=SpeedMF[RuleCon[3]].getCenterMass(DistanceMF[RuleCon[1]].getFuzzyValue(Distance))
                #print "Rule %d: Linear Speed -> rules Area: %f rules CM: %f" %(rule+1, Area, CM)
                
                SpeedArea+=Area
                SpeedMoment+=Area*CM
        try:
            speed=SpeedMoment/SpeedArea
        except:
            speed = 0
        try:
            CircularSpeed=CSpeedMoment/CSpeedArea
        except:
            CircularSpeed = 0
        #print "Moment %f Area %f" %(CSpeedMoment, CircularSpeedArea)
        return [speed, CircularSpeed] 
