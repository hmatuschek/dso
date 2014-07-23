BINARY_PATH = "../../build"
DSO_BINARY_PATH = "${BINARY_PATH}/dso/dso"

all: DSO

DSO:
	cp ${DSO_BINARY_PATH} dso.app/Contents/MacOs 
	macdeployqt dso.app
