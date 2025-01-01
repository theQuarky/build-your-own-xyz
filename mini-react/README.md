# Understanding How React Works: A Simple Implementation

This project builds a tiny version of React to understand how it works under the hood. We'll break down each part to understand how frameworks like React turn your code into what you see in the browser.

## The Big Picture

When you write React code, three main things happen:
1. You write JSX (that HTML-like code in JavaScript)
2. It gets turned into JavaScript objects (Virtual DOM)
3. These objects are then turned into real webpage elements

Let's understand each part:

## 1. Creating Elements (createElement)

```javascript
function createElement(type, props, ...children) {
    return {
        type,
        props: {
            ...props,
            children: children.map(child =>
                typeof child === "object"
                    ? child
                    : createTextElement(child)
            ),
        }
    }
}
```

Think of this function as a translator. When you write:
```jsx
<div id="foo">
    <h1>Hello</h1>
</div>
```

The function turns it into a plain JavaScript object like this:
```javascript
{
    type: "div",
    props: {
        id: "foo",
        children: [
            {
                type: "h1",
                props: {
                    children: ["Hello"]
                }
            }
        ]
    }
}
```

It's just describing your webpage structure as a regular JavaScript object. This is called the "Virtual DOM".

## 2. Handling Text (createTextElement)

```javascript
function createTextElement(text) {
    return {
        type: "TEXT_ELEMENT",
        props: {
            nodeValue: text,
            children: [],
        },
    }
}
```

This is a helper function that handles text (like "Hello" in our example). Text needs special handling because it's not a regular HTML element like a div or span.

## 3. Putting It On The Page (render)

```javascript
function render(element, container) {
    // Create the actual HTML element
    const dom =
        element.type == "TEXT_ELEMENT"
            ? document.createTextNode("")
            : document.createElement(element.type);

    // Add all properties to the element
    Object.keys(element.props || {})
        .filter(key => key !== "children")
        .forEach(name => {
            dom[name] = element.props[name]
        });

    // Create all children elements
    (element.props?.children || []).forEach(child => {
        render(child, dom);
    });
    
    // Add it to the page
    container.appendChild(dom);
}
```

This function takes our JavaScript object description and turns it into real HTML elements:
1. First, it creates the actual HTML element
2. Then it adds all the properties (like id="foo")
3. Then it does the same thing for all the children
4. Finally, it puts everything on the page

## How To Use It

1. First, we need these files:

`index.html`:
```html
<!DOCTYPE html>
<html>
<head>
    <title>Mini React</title>
    <script src="https://unpkg.com/@babel/standalone/babel.min.js"></script>
    <script src="miniReact.js"></script>
</head>
<body>
    <div id="root"></div>
    <script type="text/babel" data-type="module" src="app.js"></script>
</body>
</html>
```

`app.js`:
```jsx
/** @jsx MiniReact.createElement */
const element = (
    <div id="foo">
        <h1>bar</h1>
    </div>
);

const container = document.getElementById("root");
MiniReact.render(element, container);
```

2. Then serve it through a local server (because of how browsers work with modules)

## What's Actually Happening?

When you run this code:
1. Babel sees your JSX and turns it into `createElement` calls
2. `createElement` turns those calls into JavaScript objects
3. `render` turns those objects into real HTML elements
4. Your browser shows those elements

This is basically how React works, just much simpler. Real React adds:
- Components (reusable pieces)
- State management (making things interactive)
- Better performance
- Lots of other features

But the core idea is the same: Turn JSX into objects, then turn those objects into webpage elements.