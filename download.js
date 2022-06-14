function download(filenamePtr, dataPtr, size)
{
	const a = document.createElement('a')
	a.style = 'display:none'
	document.body.appendChild(a)
//	const view = new Uint8Array(Module.HEAPU8.buffer, dataPtr, size)
	const buffer = new Uint8Array(size)

	for (let i = 0; i < size; i++)
	{
		buffer[i] = Module.HEAPU8[dataPtr + i];
	}

	console.warn("Got here, " + buffer.length + " " + size)
	const blob = new Blob([new Uint8Array(buffer)]);//, { type: 'application/octet-stream' })
	console.warn("and to here")
	const url = window.URL.createObjectURL(blob)
	a.href = url

	const filename = UTF8ToString(filenamePtr)
	a.download = filename
	a.click()
	window.URL.revokeObjectURL(url)
	document.body.removeChild(a)
}

