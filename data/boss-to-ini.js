const btoi = require('./boss-to-ini-lib')

const fname = process.argv.find(x => x.endsWith('.boss'))
btoi.bossToIni(fname)