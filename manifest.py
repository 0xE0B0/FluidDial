from shutil import copy
import subprocess, os, sys, shutil
import hashlib

tag = (
    subprocess.check_output(["git", "describe", "--tags", "--abbrev=0"])
    .strip()
    .decode("utf-8")
)

manifestRelPath = 'release'

manifest = {
        "name": "FluidDial",
        "version": tag,
        "source_url": "https://github.com/bdring/FluidDial/tree/" + tag,
        "release_url": "https://github.com/bdring/FluidDial/releases/tag/" + tag,
        "funding_url": "https://www.paypal.com/donate/?hosted_button_id=8DYLB6ZYYDG7Y",
        "images": {},
        "files": {},
        # "upload": {
        #     "name": "upload",
        #     "description": "Things you can upload to the file system",
        #     "choice-name": "Upload group",
        #     "choices": []
        # },
        "installable": {
            "name": "installable",
            "description": "Things you can install",
            "choice-name": "Dial type",
            "choices": []
        },
}

def addImage(name, offset, filename, srcpath, dstpath):
    fulldstpath = os.path.join(manifestRelPath,os.path.normpath(dstpath))

    os.makedirs(fulldstpath, exist_ok=True)

    fulldstfile = os.path.join(fulldstpath, filename)

    shutil.copy(os.path.join(srcpath, filename), fulldstfile)

    with open(fulldstfile, "rb") as f:
        data = f.read()
    image = {
        # "name": name,
        "size": os.path.getsize(fulldstfile),
        "offset": offset,
        "path": dstpath + '/' + filename,
        "signature": {
            "algorithm": "SHA2-256",
            "value": hashlib.sha256(data).hexdigest()
        }
    }
    if manifest['images'].get(name) != None:
        print("Duplicate image name", name)
        sys.exit(1)
    manifest['images'][name] = image
    # manifest['images'].append(image)

for envName in ['m5dial', 'cyddial']:
    buildDir = os.path.join('.pio', 'build', envName)
    addImage(envName, '0x1000', 'merged-flash.bin', buildDir, envName)

def addSection(node, name, description, choice):
    section = {
        "name": name,
        "description": description,
    }
    if choice != None:
        section['choice-name'] = choice
        section['choices'] = []
    node.append(section)

def addDialType(name, description, choice=None):
    addSection(manifest['installable']['choices'], name, description, choice)

def addInstallable(install_type, erase, images):
    for image in images:
        if manifest['images'].get(image) == None:
            print("Missing image", image)
            sys.exit(1)
                      
    node1 = manifest['installable']['choices']
    installable = {
        "name": install_type["name"],
        "description": install_type["description"],
        "erase": erase,
        "images": images
    }
    node1[len(node1)-1]['choices'].append(installable)

fresh_install = { "name": "install", "description": "Complete FluidDial installation"}

def makeManifest():
    addDialType("FluidDial for M5Dial", "FluidDial for M5Dial", "FluidDial type")
    addInstallable(fresh_install, True, ["m5dial"])

    addDialType("FluidDial for CYD", "FluidDial for CYD Dial", "FluidDial type")
    addInstallable(fresh_install, True, ["cyddial"])


makeManifest()

import json
def printManifest():
    print(json.dumps(manifest, indent=2))

with open(os.path.join(manifestRelPath, "manifest.json"), "w") as manifest_file:
    json.dump(manifest, manifest_file, indent=2)
                 
sys.exit(0)
