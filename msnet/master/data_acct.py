import sys, re

def main():
        fh = None
        if len(sys.argv) != 2:
                print("Usage: %s <input_file>" % sys.argv[0])
                return
        
        try:
                fh = open(sys.argv[1], 'r')
        except IOError:
                print("Error: couldn't open log file '%s'" % sys.argv[1])
                return

        join_re = re.compile("Join from ([0-9a-f]{4}).")
        data_re = re.compile("Got data segment ([0-9a-f]{4})/([0-9]+).[0-9]+")

        next = {}
        for line in fh:
                jm = join_re.match(line)
                if jm != None:
                        node = jm.group(1)
                        next[node] = 1
                        continue
                
                dm = data_re.match(line)
                if dm != None:
                        node = dm.group(1)
                        package = int(dm.group(2))
                        if not node in next:
                                print("Unexpected data from node %s" % node)
                        elif package < next[node]:
                                print("Got repeat pkg %s from node %s, was expecting pkg %s" % (package, node, next[node]))
                        elif package > next[node]:
                                print("Got advance pkg %s from node %s, was expecting pkg %s" % (package, node, next[node]))
                                next[node] = package + 1
                        elif package == next[node]:
                                #print("Got expected pkg %s from %s" % (package, node))
                                next[node] += 1
        
        print("\nNode\tFinalPackage")
        for node, pkgs in next.items():
                print("%s\t%s" % (node, pkgs - 1))
                
if __name__ == "__main__":
        main()
