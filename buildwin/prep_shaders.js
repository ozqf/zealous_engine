'use strict';
const SOURCE_DIR = "../src/zqf_renderer/shaders/";
const OUTPUT_DIR = "../lib/shaders.h";

let fs = require("fs");
let path = require("path");
let files = fs.readdirSync(SOURCE_DIR);
files = files.filter(file => (path.extname(file) === ".glsl"));
let numFiles = files.length;
console.log(`--- Writing ${numFiles} shader files to ${OUTPUT_DIR}---`);

let shaders = [];
files.forEach(file => {
	let text = fs.readFileSync(`${SOURCE_DIR}${file}`, "utf-8");
	shaders.push(text);
});

let header = 
`#ifndef SHADERS_H
#define SHADERS_H
/* This file is automatically generated */
`;

let footer =
`
#endif // SHADERS_H
`;

let output = header;

shaders.forEach((shader, i)  => {
	output += `//////////////////////////////////////////////////\r\n`;
	output += `// ${files[i]}\r\n`;
	output += `//////////////////////////////////////////////////\r\n`;
	let name = files[i].replace(/\.[^/.]+$/, "")
	//console.log(`Writing ${name} (${shader.length} chars)`);
	output += `static const char* ${name}_text =\r\n`;
	let lines = shader.split("\r\n");
	lines.forEach(line => output += `"${line}\\n"\n`);
	output += ";\r\n";
});
output += footer;

fs.writeFileSync(OUTPUT_DIR, output);
console.log("Done");
