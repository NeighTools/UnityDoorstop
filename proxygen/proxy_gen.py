import sys
#import pefile
import io

def main():
    file = sys.argv[1]

    proxies = io.StringIO()
    proxy_add = io.StringIO()
    proxy_def = io.StringIO()

    count = 0

    """ for dll in dlls:
        pe = pefile.PE(dll, fast_load=True)
        pe.parse_data_directories()
        for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            name = exp.name.decode()

            proxies.write("%s\n" % name)
            proxy_add.write("ADD_ORIGINAL(%d, %s);\n" % (count, name))
            proxy_def.write("PROXY(%d, %s);\n" % (count, name))
            count = count + 1 """

    with open(file, "r") as includes:
        names = includes.readlines()
        for name in names:
            name = name.strip()
            proxies.write("%s\n" % name)
            proxy_add.write("ADD_ORIGINAL(%d, %s);\n" % (count, name))
            proxy_def.write("PROXY(%d, %s);\n" % (count, name))
            count = count + 1
    
    with open("templates\\dll.def", "r") as def_file:
        new_def = def_file.read().replace("{{ PROXIES }}", proxies.getvalue())

    with open("templates\\proxy_template.c", "r") as proxy_file:
        # A very dirty way to handle do template filling. Yolo for now, I suppose
        new_proxy = proxy_file.read().replace("{{ PROXY_COUNT }}", str(count)).replace("{{ PROXY_ADD }}", proxy_add.getvalue()).replace("{{ PROXY_DEF }}", proxy_def.getvalue())
    
    with open("..\\Proxy\\dll.def", "w") as dll_def_file:
        dll_def_file.write(new_def)
    with open("..\\Proxy\\proxy.c", "w") as proxy_c_file:
        proxy_c_file.write(new_proxy)

if __name__ == "__main__":
    main()