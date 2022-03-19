function _interpolate(s, tab)
    return (s:gsub('($%b{})',
                    function(w) return tab[w:sub(3, -2)] or w end))
end

function write_template(tmpl_path, out_path, replacements)
    local content = io.readfile(tmpl_path)
    io.writefile(out_path, _interpolate(content, replacements))
end