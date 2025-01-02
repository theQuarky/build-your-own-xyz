# Building Your Own Mini React: A Beginner's Guide

Hey there! 👋 Ever wondered how React works under the hood? This guide will help you understand and build your own tiny version of React! We'll keep things simple and explain everything step by step.

## Table of Contents
1. [Overview: The Big Picture](#overview-the-big-picture)
2. [Project Structure](#project-structure)
3. [Core Concepts](#core-concepts)
4. [How Each Part Works](#how-each-part-works)
5. [Building Step by Step](#building-step-by-step)
6. [Running the Project](#running-the-project)
7. [Practice Projects](#practice-projects)

## Overview: The Big Picture

When you use React, four main things happen:
1. You write JSX (HTML-like code in JavaScript)
2. This JSX becomes JavaScript objects (Virtual DOM)
3. These objects turn into real webpage elements
4. When things change, only necessary updates happen

It's like building with LEGO:
- JSX = Your building instructions
- Virtual DOM = Your building plan
- Real DOM = The actual LEGO creation
- Updates = Changing only specific pieces

## Project Structure

```
mini-react/
├── src/
│   ├── createElement.js    (JSX → Objects)
│   ├── dom.js             (Webpage Elements Handler)
│   ├── hooks.js           (State Management)
│   ├── reconciler.js      (Update Manager)
│   └── index.js           (Puts Everything Together)
├── app.js                 (Example App)
└── index.html             (Webpage)
```

## Core Concepts

### 1. Virtual DOM
Think of it as a blueprint of your webpage. Instead of directly changing the webpage (which is slow), we first make a plan of what we want to change.

Example:
```jsx
// Your JSX
<div className="box">
    <h1>Hello!</h1>
</div>

// Becomes this object (Virtual DOM)
{
    type: 'div',
    props: {
        className: 'box',
        children: [{
            type: 'h1',
            props: {
                children: ['Hello!']
            }
        }]
    }
}
```

### 2. Component System
Components are like custom LEGO pieces. You build them once and can use them anywhere:

```jsx
function Button({ onClick, children }) {
    return (
        <button onClick={onClick}>
            {children}
        </button>
    );
}

// Use it multiple times
<div>
    <Button onClick={() => alert('Hi!')}>Click Me</Button>
    <Button onClick={() => alert('Bye!')}>Bye</Button>
</div>
```

### 3. State Management
State is like a component's memory. When it changes, the component updates:

```jsx
function Counter() {
    const [count, setCount] = useState(0);
    return (
        <div>
            Count: {count}
            <button onClick={() => setCount(count + 1)}>
                Add
            </button>
        </div>
    );
}
```

## How Each Part Works

### 1. createElement (src/createElement.js)
Transforms JSX into JavaScript objects:

```javascript
function createElement(type, props, ...children) {
    return {
        type,
        props: {
            ...props,
            children: children.map(child =>
                typeof child === "object" ? child : createTextElement(child)
            )
        }
    };
}
```

Why we need it:
- Browsers don't understand JSX
- We need a standard format for our components
- Makes it easier to track changes

### 2. DOM Handler (src/dom.js)
Manages actual webpage elements:

```javascript
const DOMHandler = {
    createDOMNode(fiber) {
        // Creates actual HTML elements
    },
    updateDOMNode(node, oldProps, newProps) {
        // Updates existing elements
    },
    cleanup(node) {
        // Removes old elements and cleans up
    }
};
```

Why we need it:
- Provides a clean way to create/update elements
- Manages event listeners efficiently
- Handles cleanup to prevent memory leaks

### 3. Hooks System (src/hooks.js)
Manages component state:

```javascript
function useState(initial) {
    // Get the current component
    const component = getCurrentComponent();
    
    // Get or create state
    const state = component.state || initial;
    
    // Function to update state
    const setState = (newValue) => {
        component.state = newValue;
        updateComponent(component);
    };
    
    return [state, setState];
}
```

Why we need it:
- Components need to remember things
- Updates should trigger re-renders
- Makes components interactive

### 4. Reconciler (src/reconciler.js)
Manages efficient updates:

```javascript
function reconcileChildren(parentFiber, elements) {
    // Compare old and new elements
    // Figure out minimum changes needed
    // Schedule these changes efficiently
}
```

Why we need it:
- Updates everything would be slow
- Need to track what changed
- Makes updates smooth and efficient

## Building Step by Step

1. **Start with createElement**
   - First, make JSX work
   - Test with simple elements
   - Add support for components

2. **Add DOM Handler**
   - Create elements
   - Handle properties
   - Manage events

3. **Implement Hooks**
   - Add useState
   - Track components
   - Handle updates

4. **Build Reconciler**
   - Compare elements
   - Schedule updates
   - Apply changes

## Running the Project

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd mini-react
   ```

2. Open in browser:
   ```bash
   # Using any simple server, like Python's
   python -m http.server
   ```

3. Visit `http://localhost:8000`

## Practice Projects

Start simple and gradually add features:

1. **Counter App**
```jsx
function Counter() {
    const [count, setCount] = useState(0);
    return (
        <div>
            <h1>Count: {count}</h1>
            <button onClick={() => setCount(count + 1)}>Add</button>
        </div>
    );
}
```

2. **Todo List**
```jsx
function TodoList() {
    const [todos, setTodos] = useState([]);
    const [input, setInput] = useState('');

    return (
        <div>
            <input 
                value={input} 
                onChange={e => setInput(e.target.value)}
            />
            <button onClick={() => {
                setTodos([...todos, input]);
                setInput('');
            }}>Add Todo</button>
            <ul>
                {todos.map(todo => <li>{todo}</li>)}
            </ul>
        </div>
    );
}
```

## Debugging Tips

1. **Console Logging**
   Add logs to see what's happening:
   ```javascript
   function createElement(type, props, ...children) {
       console.log('Creating element:', { type, props, children });
       // ... rest of the function
   }
   ```

2. **Browser DevTools**
   - Inspect elements to see structure
   - Check console for errors
   - Use debugger statements

3. **Common Issues**
   - Props not updating? Check reconciliation
   - Events not working? Check event handlers
   - State not changing? Check hooks

## Remember
- Start small and build up
- Test each part as you go
- Don't worry about optimization at first
- Have fun learning!

## Need Help?
- Check the code comments
- Use console.log freely
- Break problems into smaller parts
- Ask questions!

## reference
- [build your own react](https://pomb.us/build-your-own-react/)

This project is meant for learning. Real React has many more features and optimizations, but understanding these basics will make you a better React developer!