# CRESCEDA - C++ E-Learning Terminal & Web Platform

### Why CRESCEDA?

**CRESCE + EDA**

- **Cresce** → inspired by Latin *crescere* = to grow/develop
- **EDA** → Education / Development / Academics
- **Overall concept** → growth through education

It fits your system particularly well because the platform covers the whole student journey:

**Learn → Practice → Submit → Get evaluated → Track progress → Grow**

---

A lightweight, interactive C++ E-Learning Student Platform integrated with a responsive web terminal UI. Powered by a C++ core backend (`console.cpp`) and a Node.js streaming server (`server.js`), CRESCEDA auto-compiles and pipes standard I/O (`cin`/`cout`) directly to a React web interface.

---

## Key Features

- **Student Account System**: Create student accounts with roll numbers, usernames, passwords, and semester info.
- **Course Registration**: Browse courses and register using course numbers (`1`, `2`) or course codes (`CS101`, `CS301`).
- **Assignment Submissions**: Submit assignment answers, stored systematically in organized files.
- **Quizzes & Tests**: Interactive multiple-choice testing with automatic scoring and instant evaluation.
- **Video Lecture Links**: Open course video lectures directly from the console interface.
- **Real-Time C++ Stream Bridge**: Pipes stdin and stdout in real time with ANSI color escape sequence parsing (`\033[96m`, `\033[92m`, etc.).
- **Minimalist Terminal UI**: Built with React, Vite, and Tailwind CSS.

---

## Technology Stack

- **Core Engine**: C++ (GCC `g++`)
- **Server Bridge**: Node.js, Express (`child_process.spawn`)
  - **Node.js**: Server runtime environment that manages server execution, file saving, and auto-compiles `console.cpp` using `g++`.
  - **Express**: Web server framework providing HTTP API endpoints (`GET /api/terminal` for polling terminal logs and `POST /api/terminal` for sending inputs).
  - **`child_process.spawn`**: Spawns `console.exe` as a background process and pipes real-time standard I/O (`cin` / `cout`) between C++ and the React web interface.
- **Frontend Web UI**: React, Vite, Tailwind CSS, Lucide Icons

---

## Data Organization

Application data is automatically saved in structured directories under `data/`:

```
data/
├── students/          # Student account credentials (students.txt)
├── courses/           # Course registration lists ([rollNo]_courses.txt)
└── submissions/       # Submitted assignment answers ([rollNo]_assignments.txt)
```

*(Note: The `data/` directory and compiled binaries are automatically ignored by `.gitignore` to keep your GitHub repo clean.)*

---

## Quick Start Guide

### Prerequisites
Make sure you have the following installed:
- **[Node.js](https://nodejs.org/)** (v16 or higher)
- **[GCC / g++ Compiler](https://gcc.gnu.org/)** (added to system PATH)

### Installation & Execution

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/your-username/Cresceda.git
   cd cresceda
   ```

2. **Install Dependencies**:
   ```bash
   npm install
   ```

3. **Then start the bridge server in one terminal**
  ```bash
  node server.js
  ```

4. **Then start the Vite app in a second terminal:**:
   ```bash
   npm run dev
   ```

---

## Project Structure

```
cresceda/
├── console.cpp            # C++ E-Learning core logic
├── server.js              # Express server bridge for C++ process stream
├── src/                   # React Web Terminal interface
│   ├── App.jsx            # Terminal stream parser & stdin input component
│   └── index.css          # Styling & glassmorphism theme
├── package.json           # Node project scripts & dependencies
├── vite.config.js         # Vite configuration with API proxying
├── .gitignore             # Ignores executables, node_modules, and data files
└── README.md              # Project documentation
```

