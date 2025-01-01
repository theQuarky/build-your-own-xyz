// Create the MiniReact object in the global scope
window.MiniReact = {
    createElement(type, props, ...children) {
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
    },

    createDOM(fiber) {
        const dom =
            fiber.type == "TEXT_ELEMENT"
                ? document.createTextNode("")
                : document.createElement(fiber.type);

        Object.keys(fiber.props || {})
            .filter(key => key !== "children")
            .forEach(name => {
                dom[name] = fiber.props[name]
            });

        (fiber.props?.children || []).forEach(child => {
            MiniReact.render(child, dom);
        });

        container.appendChild(dom);
    }
};
let nextUnitOfWork = null

function workLoop(deadline) {
    let shouldYield = false
    while (nextUnitOfWork && !shouldYield) {
        nextUnitOfWork = performUnitOfWork(
            nextUnitOfWork
        )
        shouldYield = deadline.timeRemaining() < 1
    }
    requestIdleCallback(workLoop)
}

requestIdleCallback(workLoop)

function performUnitOfWork(nextUnitOfWork) {
    // TODO
}

function createTextElement(text) {
    return {
        type: "TEXT_ELEMENT",
        props: {
            nodeValue: text,
            children: [],
        },
    }
}