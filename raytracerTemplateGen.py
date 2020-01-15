import jinja2

templateLoader = jinja2.FileSystemLoader(searchpath="./src/ISPC/")
templateEnv = jinja2.Environment(loader=templateLoader)
TEMPLATE_FILE = "rayTracerTemplate.ispc"
template = templateEnv.get_template(TEMPLATE_FILE)

nodeNumbers = {4,8,12,16}
leafNumbers = {4,8,12,16}
outputText = template.render( nodeNumbers = nodeNumbers, leafNumbers = leafNumbers, padding = 1)  # this is where to put args to the template renderer

#print(outputText)

# to save the results
with open("./src/ISPC/rayTracer.ispc", "w") as fh:
    fh.write(outputText)