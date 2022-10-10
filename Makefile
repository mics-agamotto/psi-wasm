source := main.cpp base64.cpp mmh3.cpp cuckoo.cpp apsi.cpp json.cpp

apsi:
	emcc -Wall -flto -O1 -sASSERTIONS -sDEMANGLE_SUPPORT -lembind libseal-3.6.a -I./ $(source) --bind -o "./dist/seal_wasm.js" -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -sNO_DISABLE_EXCEPTION_CATCHING
