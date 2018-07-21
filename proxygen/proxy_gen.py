import sys
import os
#import pefile
import io
import string

def main():
    path = os.path.dirname(os.path.realpath(sys.argv[0]))
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
            proxies.write(f"{name}\n")
            proxy_add.write(f"ADD_ORIGINAL({count}, {name});\n")
            proxy_def.write(f"PROXY({count}, {name});\n")
            count = count + 1

    with open(f"{path}\\templates\\proxy_template.c", "r") as proxy_file:
        new_proxy = string.Template(proxy_file.read()).safe_substitute(proxy_count=count, proxy_add=proxy_add.getvalue(), proxy_def=proxy_def.getvalue())
    
    with open(f"{path}\\..\\Proxy\\proxy.c", "w") as proxy_c_file:
        proxy_c_file.write(new_proxy)

if __name__ == "__main__":
    main()