import Image

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
