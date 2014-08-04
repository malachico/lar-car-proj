
class myBuffer:
    def __init__(self,size):
        self._string = ''
        self._size = size

    def add_enter(self):
        flag = False
        lastEnter = 0
        new_str = ''
        for i in range(len(self._string)):
            if self._string[i] == '\n':
                lastEnter = i
                new_str = new_str + self._string[i]
            elif i - lastEnter > self._size:
                new_str = new_str + '\n'
                lastEnter = i
                new_str = new_str + self._string[i]
            else:
                new_str = new_str + self._string[i]
        self._string = new_str

    def add_to_buffer(self,st):
        self._string += st
        self.add_enter()

'''        
import sys
buff = myBuffer(10)
buff.add_to_buffer('text\n')
buff.add_to_buffer('asdfasdfasdf')
buff.add_to_buffer('and more\n')
sys.stdout.write(buff._string)
buff.add_enter()
sys.stdout.write(buff._string)
'''
