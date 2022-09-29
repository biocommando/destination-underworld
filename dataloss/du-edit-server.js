const http = require('http')
const fs = require('fs')

const port = 3000

const mimeTypeMap = {'.aac': 'audio/aac','.abw': 'application/x-abiword','.arc': 'application/x-freearc','.avi': 'video/x-msvideo','.azw': 'application/vnd.amazon.ebook','.bin': 'application/octet-stream','.bmp': 'image/bmp','.bz': 'application/x-bzip','.bz2': 'application/x-bzip2','.csh': 'application/x-csh','.css': 'text/css','.csv': 'text/csv','.doc': 'application/msword','.docx': 'application/vnd.openxmlformats-officedocument.wordprocessingml.document','.eot': 'application/vnd.ms-fontobject','.epub': 'application/epub+zip','.gz': 'application/gzip','.gif': 'image/gif','.htm': 'text/html', '.html': 'text/html','.ico': 'image/vnd.microsoft.icon','.ics': 'text/calendar','.jar': 'application/java-archive','.jpeg': 'image/jpeg', '.jpg': 'image/jpeg','.js': 'text/javascript','.json': 'application/json','.jsonld': 'application/ld+json','.mid': 'audio/midi audio/x-midi', '.midi': 'audio/midi audio/x-midi','.mjs': 'text/javascript','.mp3': 'audio/mpeg','.mpeg': 'video/mpeg','.mpkg': 'application/vnd.apple.installer+xml','.odp': 'application/vnd.oasis.opendocument.presentation','.ods': 'application/vnd.oasis.opendocument.spreadsheet','.odt': 'application/vnd.oasis.opendocument.text','.oga': 'audio/ogg','.ogv': 'video/ogg','.ogx': 'application/ogg','.opus': 'audio/opus','.otf': 'font/otf','.png': 'image/png','.pdf': 'application/pdf','.php': 'application/php','.ppt': 'application/vnd.ms-powerpoint','.pptx': 'application/vnd.openxmlformats-officedocument.presentationml.presentation','.rar': 'application/x-rar-compressed','.rtf': 'application/rtf','.sh': 'application/x-sh','.svg': 'image/svg+xml','.swf': 'application/x-shockwave-flash','.tar': 'application/x-tar','.tif': 'image/tiff', '.tiff': 'image/tiff','.ts': 'video/mp2t','.ttf': 'font/ttf','.txt': 'text/plain','.vsd': 'application/vnd.visio','.wav': 'audio/wav','.weba': 'audio/webm','.webm': 'video/webm','.webp': 'image/webp','.woff': 'font/woff','.woff2': 'font/woff2','.xhtml': 'application/xhtml+xml','.xls': 'application/vnd.ms-excel','.xlsx': 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet','.xml': 'application/xml','.xul': 'application/vnd.mozilla.xul+xml','.zip': 'application/zip','.3gp': 'video/3gpp','.3g2': 'video/3gpp2', '.7z': 'application/x-7z-compressed',}

let missionPack = process.argv.find(x => x.startsWith('--mission-pack='))
if (!missionPack) missionPack = 'core-pack'
else missionPack = missionPack.split('=')[1]

http.createServer((req,res) => {
    try {
        let url = req.url
        url = url.split('?')[0]
        console.log('requested', url)
        if (url.split('/')[1] === 'api') {
            const cmd = url.split('/')[2]
            const param = url.split('/')[3]
            if (cmd === 'load') {
                url = '/' + missionPack + '/mission' + param
            }
            else if (cmd === 'save') {
                let body = ''
                req.on('data', data => body += data)
                req.on('end', () => {
                    fs.writeFileSync(missionPack + '/mission' + param, body)
                    res.end('ok')
                })
                return
            }
            else
            {
                res.end('unknown command')
            }
        }
        if (url === '/') url = '/du-edit.html'
        const extension = '.' + url.replace(/.+\./, '')
        let mimeType = mimeTypeMap[extension]
        if (!mimeType) mimeType = 'text/plain'
        res.writeHead(200, {'Content-Type': mimeType})
        res.end(fs.readFileSync(url.substr(1)))
    } catch(e) {
        console.log('error', e)
        res.end()
    }
}).listen(port)

console.log('Started server at port %d', port)