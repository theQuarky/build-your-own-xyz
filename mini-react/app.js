/** @jsx MiniReact.createElement */
const fiber = (
    <div id="foo">
        <h1>bar</h1>
        {/* <input type="text"/>
        <ul>
            <li>1</li>
            <li>2</li>
            <li>3</li>
            <li>4</li>
        </ul>
        <b /> */}
    </div>
);

const container = document.getElementById("root");
MiniReact.render(fiber, container);