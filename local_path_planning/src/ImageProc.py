import Image
import numpy as np
import colorsys

rgb_to_hsv = np.vectorize(colorsys.rgb_to_hsv)
hsv_to_rgb = np.vectorize(colorsys.hsv_to_rgb)
def get_color(arr):
    r = []
    g = []
    b = []
    a = []
    for pix in arr:
        r.append(pix[0])
        g.append(pix[1])
        b.append(pix[2])
        a.append(pix[3])
    return r,g,b,a

def matrixify(arr,length,width):
    ar = []
    temp = []
    j=0
    for pix in arr:
        j += 1
        temp.append(pix)
        if (j==width):
            ar.append(temp)
            temp = []
            j = 0
    return ar
#arr = [1,2,3,4,5,6,7,8,9]
#print matrixify(arr,3,3)

def shift_hue(arr, hout):
    print len(arr),len(arr[0])
    r, g, b,a = get_color(arr)#np.rollaxis(arr, axis=-1)#
    print "r: ", r
    print g
    print b
    print 'a: ',a
    h, s, v = rgb_to_hsv(r, g, b)
    h = hout
    r, g, b = hsv_to_rgb(h, s, v)
    arr = np.dstack((r, g, b, a))
    return arr

def rgb2hsv(im,width=543,height=922):
    new_im = np.zeros((width,height,3))
    #height = new_im.get_width()
    i=0
    j=0
    for pix in list(im.getdata()):
        if j==height:
            j = 0
            i += 1
        h,s,v = (colorsys.rgb_to_hsv(pix[0],pix[1],pix[2]))
        new_im[i,j,0] = h
        new_im[i,j,1] = s
        new_im[i,j,2] = v
        j += 1
    return new_im

def hsv2rgb(im,width=543,height=922):
    new_im = np.zeros((width,height,3))
    i=0
    j=0
    for row in im:
        for pix in row:
            if j==height:
                j=0
                i += 1
            r,g,b = colorsys.hsv_to_rgb(pix[0],pix[1],pix[2])
            new_im[i,j,0] = r
            new_im[i,j,1] = g
            new_im[i,j,2] = b
    #new_im = np.array(new_im)
    #new_im.resize(width,height)
    new_im = Image.fromarray(new_im, 'RGBA')
    #print len(new_im),len(new_im[0])
    #for pix in new_im:
    return new_im

def colorize(image, hue):
    """
    Colorize PIL image `original` with the given
    `hue` (hue within 0-360); returns another PIL image.
    """
    img = image.convert('RGBA')
    arr = list(img.getdata())#np.array(np.asarray(img).astype('float'))#
    arr = matrixify(arr,678,1206)
    #new_img = Image.fromarray(shift_hue(arr, hue/360.).astype('uint8'), 'RGBA')
    new_img = Image.fromarray(np.array(shift_hue(arr,hue/360)).astype('uint8'),'RGBA')
    return new_img

#im = Image.open('gazebo.png')
#im.show()

#im_hsv = rgb2hsv(im)#colorsys.rgb_to_hsv(im)
#im_rgb = hsv2rgb(im_hsv)
#im_rgb.show()
#new_im = colorize(im,300)
#new_im.show()
def is_adj(arr,I,J):
    try:
        for i in range(-4,5):
            for j in range(-4,5):
                if arr[I+i][J+j] == 255:
                    return 0
        return 1
    except:
        return 0

def vec2im(arr,Sx=40,Sy=25):
    im = []
    for i in range(0,Sy):
        row = []
        for j in range(0,Sx):
            row.append(arr[i*Sx+j])
        im.append(row)
    return im
def find_position(arr,Sx=40,Sy=25):
    im = vec2im(arr)
    min = -1
    max = -1
    for i in range(0,Sy):
        for j in range(0,Sx):
            if (is_adj(im,i,j)):
                if min==-1:
                    min = j
                max = j
        if not min==-1:
            break
    if abs(min-Sx)>max:
        return [max,i]
    return [min,i]
