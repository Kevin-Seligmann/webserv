// Remover testDelete(), ya no se necesita
function testPut() {
    fetch('/test.txt', {
        method: 'PUT',
        headers: {'Content-Type': 'text/plain'},
        body: 'Content created via PUT method'
    })
    .then(response => response.text())
    .then(data => {
        document.getElementById('result').innerHTML = 
            '<p>PUT Response: ' + data + '</p>';
    })
    .catch(error => {
        document.getElementById('result').innerHTML = 
            '<p>Error: ' + error + '</p>';
    });
}