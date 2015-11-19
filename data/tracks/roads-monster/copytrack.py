f = open('template.track')
m = open('map2.track','w');
lines = f.read().split('\n')
for line in lines:
    m.write(line + line + "\n")
f.close()
m.close()