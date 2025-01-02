/** @jsx MiniReact.createElement */
function Counter() {
    const [state, setState] = MiniReact.useState(1);

    return (
        <div className="counter">
            <h1>Count: {state}</h1>
            <button
                onClick={() => setState(state => state - 1)}
                className="button"
            >
                Decrease
            </button>
            <button
                onClick={() => setState(state => 0)}
                className="button"
            >
                Reset
            </button>
            <button
                onClick={() => setState(state => state + 1)}
                className="button"
            >
                Increase
            </button>
            <br />
            <input type="range" onChange={(e) => {
                setState(+e.target.value);
            }} />
        </div>
    );
}

// Create and render the app
const element = <Counter />;
const container = document.getElementById("root");
MiniReact.render(element, container);
