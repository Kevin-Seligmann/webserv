function testDelete() {
    fetch('/upload/test.txt', {
        method: 'DELETE',
    })
    .then(response => {
        document.getElementById('result').innerHTML = 
            `DELETE request sent. Status: ${response.status}`;
    })
    .catch(error => {
        document.getElementById('result').innerHTML = 
            `DELETE request failed: ${error}`;
    });
}

function testPut() {
    fetch('/upload/test.txt', {
        method: 'PUT',
        body: 'Test content for PUT request',
        headers: {
            'Content-Type': 'text/plain'
        }
    })
    .then(response => {
        document.getElementById('result').innerHTML = 
            `PUT request sent. Status: ${response.status}`;
    })
    .catch(error => {
        document.getElementById('result').innerHTML = 
            `PUT request failed: ${error}`;
    });
}