//Require Stuff
const uuid = require('uuid').v4;
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const raspi = require('raspi');
const I2C = require('raspi-i2c').I2C;


//Config Variables
const serverPort = 3000;

//Global Variables
let vent = {
    uuid: uuid(),
    address: 0x04,
    state: 'open',
    alive: true
};
const app = express();
const i2c = new I2C();
let commandQueue = [];

//Function Declarations

//Helper Functions
function refreshVents() {
    //Pings all vents, checks if they are still alive. Removes any that aren't.
    
}

//Vent Interfact Functions
async function pingVent(address) {
    //Return promise which resolves to true/false value.
    await i2c.writeSync(address, undefined, Buffer.from('a'));
    console.log('ping!');
    let response = await responseBuf.readSync(address, undefined, 0x04);
    if (response = 2) console.log('Pong!');
    return true;
};

function movement(address, state) {
    //Return promise which completes once vent has moved.
    i2c.writeSync(address, undefined, Buffer.from((state == 'open') ? 'b' : 'c'));
    console.log('Vent toggled!');
};

function checkVent(address) {
    //Check status of vent.
};

function resetVent(address) {
    //Reset a vent if an error has occured.
    i2c.writeSync(address, undefined, Buffer.from('e'));
}


//Express API endpoints

app.use(cors());
app.use(bodyParser.urlencoded( { extended: false } ));
app.use(bodyParser.json());

app.post('/openVent', (req, res) => {
    let exampleCommand = {command: 'open'};
    const command = req.body;
    console.log(ventCommands);
    if (command.command === 'open') {
        console.log('Opening Vent')
        movement(vent.address, 'open');
    } else if (command.command === 'close') {
        console.log('Closing Vent');
        movement(vent.address, 'close');
    } else if (command.command === 'ping') {
        console.log('Pinging Vent');
        pingVent(vent.address);
    } else if (command.command = 'resetError') {
        console.log('Resetting Error');
        resetVent(vent.address);
    } else {
        console.log('Unknown Command Recieved.')
    }
    res.send(`Succesfully Received: \n ${JSON.stringify(req.body)}`);
});

app.get('/vents', (req, res) => {
    res.json(vents);
});

app.get('/', (req, res) => {
    res.header('Vent Control System');
    res.statusCode = 404;
    res.send('This is the control server for vent valves under the house. Designed and installed by Josiah Bull.')
});

app.listen(serverPort, () => {
    console.log(`Server running on port: ${serverPort}`);
});