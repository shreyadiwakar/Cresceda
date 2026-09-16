import React, { useState, useEffect, useRef } from 'react';
import { Send, RotateCcw, Trash2 } from 'lucide-react';

// Process ANSI escape codes and parse color tokens for continuous terminal stream
function processTerminalStream(rawText) {
  if (!rawText) return [];

  // Replace ANSI clear screen codes (\033[2J\033[H) with clean line breaks so history stays visible
  const cleanStr = rawText
    .replace(/\r/g, '')
    .replace(/(\u001b|\033)?\[2J(\u001b|\033)?\[H|\u001b\[CLEAR]/g, '\n\n');

  const tokens = [];
  const regex = /(?:\u001b|\033)\[(\d+)m/g;
  let lastIndex = 0;
  let currentColor = 'text-slate-200 font-normal';

  let match;
  while ((match = regex.exec(cleanStr)) !== null) {
    const plainText = cleanStr.substring(lastIndex, match.index);
    if (plainText) {
      tokens.push({ text: plainText, className: currentColor });
    }

    const code = match[1];
    switch (code) {
      case '96': currentColor = 'text-cyan-400 font-bold'; break;
      case '92': currentColor = 'text-emerald-400 font-bold'; break;
      case '91': currentColor = 'text-rose-400 font-bold'; break;
      case '93': currentColor = 'text-yellow-400 font-bold'; break;
      case '0': currentColor = 'text-slate-200 font-normal'; break;
      default: currentColor = 'text-slate-200 font-normal'; break;
    }
    lastIndex = regex.lastIndex;
  }

  const remainingText = cleanStr.substring(lastIndex);
  if (remainingText) {
    tokens.push({ text: remainingText, className: currentColor });
  }

  return tokens;
}

// Helper to render terminal tokens with clickable HTTP/HTTPS links
function renderFormattedToken(tok, key) {
  const urlRegex = /(https?:\/\/[^\s\r\n]+)/g;
  const parts = tok.text.split(urlRegex);
  if (parts.length <= 1) {
    return <span key={key} className={tok.className}>{tok.text}</span>;
  }

  return (
    <span key={key} className={tok.className}>
      {parts.map((part, pIdx) => {
        if (/^https?:\/\//i.test(part)) {
          return (
            <a
              key={pIdx}
              href={part}
              target="_blank"
              rel="noopener noreferrer"
              className="underline text-cyan-300 hover:text-cyan-100 cursor-pointer font-semibold bg-cyan-950/50 px-1 py-0.5 rounded transition-colors"
            >
              {part}
            </a>
          );
        }
        return part;
      })}
    </span>
  );
}

export default function App() {
  const [terminalText, setTerminalText] = useState('');
  const [inputValue, setInputValue] = useState('');
  const [isServerActive, setIsServerActive] = useState(false);
  const logEndRef = useRef(null);
  const processedVideoCountRef = useRef(0);

  // Poll for terminal output from console.exe via Node server bridge
  const fetchTerminalOutput = async () => {
    try {
      const res = await fetch('/api/terminal');
      if (res.ok) {
        const data = await res.json();
        setIsServerActive(true);
        if (data.output !== undefined) {
          setTerminalText(data.output);
        }
      }
    } catch (e) {
      setIsServerActive(false);
    }
  };

  useEffect(() => {
    fetchTerminalOutput();
    const interval = setInterval(() => fetchTerminalOutput(), 300);

    return () => {
      clearInterval(interval);
    };
  }, []);

  // Auto-open video links in a new browser tab when printed in terminal
  useEffect(() => {
    if (!terminalText) {
      processedVideoCountRef.current = 0;
      return;
    }
    const regex = /Opening video:\s*(https?:\/\/[^\s\r\n]+)/gi;
    const matches = [...terminalText.matchAll(regex)];
    if (matches.length > processedVideoCountRef.current) {
      for (let i = processedVideoCountRef.current; i < matches.length; i++) {
        const url = matches[i][1];
        try {
          window.open(url, '_blank', 'noopener,noreferrer');
        } catch (e) {
          console.error('Failed to auto-open URL:', e);
        }
      }
      processedVideoCountRef.current = matches.length;
    }
  }, [terminalText]);

  // Auto-scroll terminal window to bottom
  useEffect(() => {
    logEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [terminalText]);

  // Send stdin input to console.exe
  const handleSendInput = async (e) => {
    e.preventDefault();
    const inputStr = inputValue;
    setInputValue('');

    // Append input command prompt to local output display
    setTerminalText(prev => prev + `\n> ${inputStr}\n`);

    try {
      const res = await fetch('/api/terminal', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ input: inputStr }),
      });
      if (res.ok) {
        const data = await res.json();
        if (data.output) {
          setTerminalText(prev => prev + data.output);
        }
      }
    } catch (e) {
      setTerminalText(prev => prev + '\n[Error communicating with C++ console process]\n');
    }
  };

  const handleRestart = async () => {
    setTerminalText('');
    try {
      const res = await fetch('/api/terminal', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ input: 'restart' }),
      });
      if (res.ok) {
        const data = await res.json();
        if (data.output) {
          setTerminalText(data.output);
        }
      }
    } catch (e) {}
  };

  const tokens = processTerminalStream(terminalText);

  return (
    <div className="min-h-screen bg-[#0b0f19] text-slate-100 font-sans flex flex-col items-center justify-center p-3 sm:p-6 selection:bg-pink-500/30 selection:text-pink-300">
      <div className="w-full max-w-4xl space-y-4 sm:space-y-6">
        
        {/* Minimal Responsive Heading */}
        <div className="text-center space-y-1">
          <h1 className="text-2xl sm:text-4xl font-extrabold tracking-tight bg-gradient-to-r from-pink-400 via-purple-300 to-cyan-300 bg-clip-text text-transparent">
            CRESCEDA
          </h1>
          <div className="flex items-center justify-center space-x-2 font-mono text-[11px] sm:text-xs text-slate-400">
            <span>C++ Stream Engine</span>
            <span className={`w-2 h-2 rounded-full ${isServerActive ? 'bg-emerald-400 animate-pulse' : 'bg-amber-500'}`}></span>
          </div>
        </div>

        {/* Responsive Terminal Frame */}
        <div className="cute-glass rounded-2xl overflow-hidden border border-slate-800/90 shadow-2xl bg-[#131b2e]/90 w-full">
          
          {/* Title Bar */}
          <div className="bg-slate-900/90 px-3 sm:px-4 py-2.5 sm:py-3 border-b border-slate-800 flex items-center justify-between">
            <div className="flex items-center space-x-2">
              <div className="w-3 h-3 rounded-full bg-pink-500"></div>
              <div className="w-3 h-3 rounded-full bg-yellow-500"></div>
              <div className="w-3 h-3 rounded-full bg-cyan-500"></div>
              <span className="text-[11px] sm:text-xs font-mono text-slate-400 ml-1.5 truncate">cresceda ~ terminal</span>
            </div>

            <div className="flex items-center space-x-1 sm:space-x-2">
              <button
                onClick={handleRestart}
                className="p-1.5 rounded-lg hover:bg-slate-800 text-slate-400 hover:text-white transition-colors"
                title="Restart C++ Process"
              >
                <RotateCcw className="w-3.5 h-3.5" />
              </button>
              <button
                onClick={() => setTerminalText('')}
                className="p-1.5 rounded-lg hover:bg-slate-800 text-slate-400 hover:text-white transition-colors"
                title="Clear screen"
              >
                <Trash2 className="w-3.5 h-3.5" />
              </button>
            </div>
          </div>

          {/* Responsive Terminal Output Stream */}
          <div className="h-[55vh] min-h-[320px] max-h-[520px] p-4 sm:p-6 font-mono text-xs sm:text-sm overflow-y-auto overflow-x-hidden leading-relaxed whitespace-pre-wrap break-words bg-slate-950">
            {tokens.length > 0 ? (
              tokens.map((tok, i) => renderFormattedToken(tok, i))
            ) : (
              <span className="text-slate-500 italic">Connecting to console.exe stream...</span>
            )}
            <div ref={logEndRef} />
          </div>

          {/* Stdin Input Box */}
          <form 
            onSubmit={handleSendInput}
            className="bg-slate-900/95 p-2.5 sm:p-3 border-t border-slate-800 flex items-center space-x-2"
          >
            <span className="text-pink-400 font-mono font-bold text-sm pl-1 sm:pl-2">&gt;</span>
            <input
              type="text"
              value={inputValue}
              onChange={(e) => setInputValue(e.target.value)}
              placeholder="Type choice or value into C++ stdin..."
              className="flex-1 bg-slate-950 px-3 sm:px-4 py-2 sm:py-2.5 rounded-xl text-xs sm:text-sm font-mono text-cyan-300 placeholder-slate-500 border border-slate-800 focus:outline-none focus:border-pink-500/50"
              autoFocus
            />
            <button
              type="submit"
              className="px-3.5 sm:px-4 py-2 sm:py-2.5 rounded-xl bg-gradient-to-r from-pink-500 to-purple-600 hover:from-pink-400 hover:to-purple-500 text-slate-950 font-bold text-xs sm:text-sm shadow-md shadow-pink-500/20 flex items-center space-x-1.5 transition-all"
            >
              <span className="hidden sm:inline">Send Input</span>
              <Send className="w-3.5 h-3.5" />
            </button>
          </form>

        </div>

      </div>
    </div>
  );
}
