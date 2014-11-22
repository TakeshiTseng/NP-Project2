from os import listdir

total = 0
for file_name in listdir('.'):
    if ".c" in file_name or ".h" in file_name:
        f = open(file_name)
        file_line = 0
        for line in f:
            if len(line) > 0:
                file_line += 1
                total += 1

        print "%s %d" % (file_name, file_line)

print "total", total
