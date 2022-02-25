let wasmainG = null;
let memoryG = null;
let decoder = new TextDecoder('utf8');

async function load_bfli() {
	const response = await fetch('bfli.wasm');
	const bytes = await response.arrayBuffer();
	const results = await WebAssembly.instantiate(bytes, {});
	const { instance: { exports: { memory, wasmain } } } = results;
	memoryG = memory;
	wasmainG = wasmain;
}

function bfli_main(input) {
	const len = input.length;
	let output = new Uint8Array(memoryG.buffer, 0, 32768);
	output.set([0]);
	const input_cstr = new Uint8Array(memoryG.buffer, output.length, len)
	const input_u8 = [];
	for (var i = 0; i < len; ++i) { input_u8.push(input.charCodeAt(i)); }
	input_cstr.set(input_u8);
	// console.log(input_cstr);
	wasmainG(input_cstr.byteOffset, output.byteOffset, len);
	document.getElementById("bfli_out").innerHTML = decoder.decode(output);
}

load_bfli();