import express from 'express';
import cors from 'cors';
import { spawn, execSync } from 'child_process';
import path from 'path';
import fs from 'fs';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const app = express();

app.use(cors());
app.use(express.json());

let cppProcess = null;
let fullHistory = '';

function startCppProcess() {
  if (cppProcess) {
    try { cppProcess.kill(); } catch (e) {}
  }

  // Forcefully terminate any running console.exe locks on Windows
  if (process.platform === 'win32') {
    try { execSync('taskkill /F /IM console.exe 2>nul', { stdio: 'ignore' }); } catch (e) {}
  }

  fullHistory = '';
  const exePath = path.join(__dirname, 'console.exe');
  const cppPath = path.join(__dirname, 'console.cpp');

  // Auto-compile console.cpp if exe missing or code modified
  try {
    if (!fs.existsSync(exePath) || (fs.existsSync(cppPath) && fs.statSync(cppPath).mtime > fs.statSync(exePath).mtime)) {
      console.log('Compiling console.cpp...');
      execSync('g++ -Wall -Wextra console.cpp -o console.exe', { cwd: __dirname });
      console.log('Compilation complete!');
    }
  } catch (err) {
    console.error('Compilation error:', err.message);
  }

  cppProcess = spawn(exePath, [], {
    cwd: __dirname,
    env: { ...process.env, TERM: 'xterm-256color' }
  });

  cppProcess.stdout.on('data', (data) => {
    fullHistory += data.toString();
  });

  cppProcess.stderr.on('data', (data) => {
    fullHistory += data.toString();
  });

  cppProcess.on('exit', () => {
    cppProcess = null;
  });
}

// Start C++ process on launch
startCppProcess();

// Poll for terminal logs
app.get('/api/terminal', (req, res) => {
  if (!cppProcess) {
    startCppProcess();
    setTimeout(() => {
      res.json({ output: fullHistory, active: !!cppProcess });
    }, 300);
    return;
  }

  res.json({ output: fullHistory, active: !!cppProcess });
});

// Send stdin to C++ process
app.post('/api/terminal', (req, res) => {
  const { input } = req.body;

  if (input === 'restart' || input === 'reset' || !cppProcess) {
    startCppProcess();
    setTimeout(() => {
      res.json({ output: fullHistory, active: true });
    }, 300);
    return;
  }

  if (cppProcess && input !== undefined) {
    fullHistory += `\n> ${input}\n`;
    cppProcess.stdin.write(input + '\n');
  }

  setTimeout(() => {
    res.json({ output: fullHistory, active: !!cppProcess });
  }, 200);
});

const PORT = 3002;
app.listen(PORT, () => {
  console.log(`C++ Stream Server running on http://127.0.0.1:${PORT}`);
});
