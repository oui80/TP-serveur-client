f= open("./data_serveur/test.txt","a")
for n in range(10000):
    f.write(str(n))
f.close() 