# 🎮 Advanced Tic Tac Toe Game

<div align="center">

![Tic Tac Toe Banner](https://img.shields.io/badge/Game-Tic%20Tac%20Toe-blue?style=for-the-badge&logo=gamepad)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-41CD52?style=for-the-badge&logo=qt&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-07405E?style=for-the-badge&logo=sqlite&logoColor=white)

[![Build Status](https://img.shields.io/github/workflow/status/yourusername/tictactoe/CI?style=flat-square)](https://github.com/yourusername/tictactoe/actions)
[![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)](LICENSE)
[![Release](https://img.shields.io/github/v/release/yourusername/tictactoe?style=flat-square)](https://github.com/yourusername/tictactoe/releases)

*A feature-rich Tic Tac Toe game with AI opponents, user authentication, and comprehensive game history tracking.*

[🚀 Quick Start](#-quick-start) • [📸 Screenshots](#-screenshots) • [🎯 Features](#-features) • [🛠️ Installation](#️-installation) • [📖 Documentation](#-documentation)

</div>

---

## 📸 Screenshots

<div align="center">
<img src="assets/game-interface.png" alt="Game Interface" width="300"/>
<img src="assets/login-screen.png" alt="Login Screen" width="300"/>
<img src="assets/game-history.png" alt="Game History" width="300"/>
</div>

## 🎯 Features

### 🎮 Game Modes
- **👥 Player vs Player** - Challenge your friends locally
- **🤖 Player vs AI** - Test your skills against intelligent AI opponents
- **🎲 Symbol Selection** - Choose to play as X or O

### 🧠 AI Intelligence Levels
- **🟢 Easy** - Random move selection, perfect for beginners
- **🟡 Medium** - Strategic play with win/block detection
- **🔴 Hard** - Unbeatable Minimax algorithm implementation

### 🔐 User Management
- **📝 User Registration** - Create secure accounts with hashed passwords
- **🔑 User Authentication** - Secure login system
- **👤 Multi-user Support** - Multiple players can maintain separate profiles

### 📊 Game Analytics
- **📈 Game History Tracking** - Complete record of all matches
- **🏆 Win/Loss Statistics** - Track your performance over time
- **📅 Timestamped Records** - See when each game was played
- **🎯 Result Categories** - Wins, losses, and draws against AI or players

### 🛡️ Security Features
- **🔒 SQL Injection Protection** - Parameterized queries and input sanitization
- **🔐 Password Hashing** - Secure password storage
- **✅ Input Validation** - Comprehensive error handling

### 💻 Interface Options
- **🖥️ Command Line Interface** - Full-featured console version
- **🖼️ Qt GUI Application** - Modern graphical user interface
- **📱 Cross-platform Support** - Works on Windows, macOS, and Linux

## 🚀 Quick Start

### Prerequisites
```bash
# Required dependencies
- C++17 or later
- SQLite3
- Qt5/Qt6 (for GUI version)
- CMake 3.16+
```

### 🔧 Installation

#### Option 1: Using releases (Recommended)
```bash
# Download the latest release
wget https://github.com/yourusername/tictactoe/releases/latest/download/tictactoe-linux.tar.gz

# Extract and run
tar -xzf tictactoe-linux.tar.gz
cd tictactoe
./tictactoe
```

#### Option 2: Build from source
```bash
# Clone the repository
git clone https://github.com/yourusername/tictactoe.git
cd tictactoe

# Build console version
mkdir build && cd build
cmake ..
make

# Run console version
./tictactoe

# Build GUI version (if Qt is installed)
cmake -DBUILD_GUI=ON ..
make
./tictactoe-gui
```

#### Option 3: Docker
```bash
# Run with Docker
docker run -it yourusername/tictactoe:latest
```

## 📖 How to Play

### 🎯 Game Rules
1. **Objective**: Get three of your symbols (X or O) in a row, column, or diagonal
2. **Turns**: Players alternate placing their symbols on the 3x3 grid
3. **Winning**: First to achieve three in a row wins
4. **Draw**: If all spaces are filled without a winner, it's a draw

### 🎮 Controls
- **Console**: Enter row and column coordinates (0-2)
- **GUI**: Click on desired grid position

### 🤖 AI Difficulty Guide
| Level | Description | Strategy |
|-------|-------------|----------|
| 🟢 **Easy** | Random moves | Good for learning |
| 🟡 **Medium** | Smart moves + blocking | Balanced challenge |
| 🔴 **Hard** | Minimax algorithm | Nearly impossible to beat |

## 🏗️ Architecture

### 📁 Project Structure
```
tictactoe/
├── 📂 src/
│   ├── 📄 main.cpp              # Console application entry point
│   ├── 📄 game.cpp              # Core game logic
│   ├── 📄 database.cpp          # SQLite database operations
│   ├── 📄 ai.cpp                # AI algorithms
│   └── 📄 auth.cpp              # Authentication system
├── 📂 gui/
│   ├── 📄 mainwindow.cpp        # Qt main window
│   ├── 📄 gamewidget.cpp        # Game board widget
│   └── 📄 *.ui                  # UI files
├── 📂 tests/
│   ├── 📄 test_game.cpp         # Game logic tests
│   ├── 📄 test_ai.cpp           # AI algorithm tests
│   └── 📄 test_database.cpp     # Database tests
├── 📂 assets/                   # Images and resources
├── 📂 docs/                     # Documentation
├── 📄 CMakeLists.txt            # Build configuration
├── 📄 .github/workflows/        # CI/CD pipelines
└── 📄 README.md                 # This file
```

### 🔧 Core Components

#### 🎮 Game Engine
- **Board Management**: 3x3 grid state tracking
- **Move Validation**: Input sanitization and rule enforcement
- **Win Detection**: Efficient algorithm for checking game end conditions

#### 🤖 AI System
- **Easy AI**: Random move selection from available positions
- **Medium AI**: Priority-based decision making (win → block → random)
- **Hard AI**: Minimax algorithm with alpha-beta pruning for optimal play

#### 💾 Database Layer
- **SQLite Integration**: Lightweight, serverless database
- **User Management**: Secure authentication and session handling
- **Game History**: Comprehensive match recording and retrieval

## 🛠️ Development

### 🔨 Building

#### Console Version
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### GUI Version
```bash
mkdir build && cd build
cmake -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### Development Build
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON ..
make -j$(nproc)
```

### 🧪 Testing
```bash
# Run all tests
ctest --verbose

# Run specific test suite
./tests/test_game
./tests/test_ai
./tests/test_database
```

### 📊 Code Coverage
```bash
# Generate coverage report
cmake -DENABLE_COVERAGE=ON ..
make coverage
```

## 🚀 CI/CD Pipeline

### 🔄 Continuous Integration
Our CI pipeline automatically:
- ✅ **Builds** on multiple platforms (Linux, Windows, macOS)
- 🧪 **Tests** all components with comprehensive test suite
- 📊 **Analyzes** code quality and coverage
- 🛡️ **Scans** for security vulnerabilities
- 📦 **Packages** release artifacts

### 🚀 Continuous Deployment
- 🏷️ **Automatic releases** on version tags
- 📦 **Multi-platform binaries** (Linux, Windows, macOS)
- 🐳 **Docker images** pushed to registry
- 📚 **Documentation updates** deployed to GitHub Pages

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### 🐛 Bug Reports
- Use the [issue tracker](https://github.com/yourusername/tictactoe/issues)
- Include system information and steps to reproduce
- Attach relevant logs or screenshots

### ✨ Feature Requests
- Check existing [feature requests](https://github.com/yourusername/tictactoe/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement)
- Provide detailed use cases and requirements
- Consider contributing the implementation

### 🔧 Development Setup
```bash
# Fork and clone the repository
git clone https://github.com/yourusername/tictactoe.git
cd tictactoe

# Install development dependencies
./scripts/setup-dev.sh

# Create feature branch
git checkout -b feature/your-feature-name

# Make changes and test
make test

# Submit pull request
```

## 📋 Roadmap

### 🎯 Version 2.0
- [ ] 🌐 Network multiplayer support
- [ ] 🎨 Customizable themes and skins
- [ ] 🔊 Sound effects and music
- [ ] 📱 Mobile app versions

### 🎯 Version 2.1
- [ ] 🏆 Tournament mode
- [ ] 📊 Advanced statistics and analytics
- [ ] 🤖 Machine learning AI improvements
- [ ] 🌍 Internationalization support

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors

- **Your Name** - *Initial work* - [YourGitHub](https://github.com/yourusername)

See also the list of [contributors](https://github.com/yourusername/tictactoe/contributors) who participated in this project.

## 🙏 Acknowledgments

- 🎮 Inspired by the classic Tic Tac Toe game
- 🤖 AI algorithms based on game theory principles
- 🛡️ Security best practices from OWASP guidelines
- 🎨 UI/UX inspired by modern game design

## 📞 Support

- 📧 **Email**: support@yourproject.com
- 💬 **Discord**: [Join our community](https://discord.gg/yourserver)
- 📖 **Documentation**: [Full documentation](https://yourusername.github.io/tictactoe)
- 🐛 **Issues**: [GitHub Issues](https://github.com/yourusername/tictactoe/issues)

---

<div align="center">

**⭐ Star this repository if you found it helpful! ⭐**

Made with ❤️ by [Your Name](https://github.com/yourusername)

</div>
