const recordsElement = document.getElementById('records');


window.onload = () => {
    fetchrecords();
}


const apiUrl = "http://localhost:6969";
const callAPI = async (payload) => {
    console.log('Calling API with payload:', payload);
    console.log('looks like this: ', payload);
    try {
        const response = await fetch(apiUrl,
            {
                method: 'POST',
                body: payload
            });
        const data = await response.json();
        return data;
    } catch (error) {
        console.error("Error calling API:", error);
    }
}



const fetchrecords = async () => {
    try {
        const data = await callAPI( 'function=get_all_records' ); //function=get_all_records
        recordsElement.innerHTML = '';
        console.log("Fetched records:", data);

        if (data && data.length > 0) {
            for (let i = 0; i < data.length; i++) {
                const record = data[i];
                const recordDiv = document.createElement('div');
                recordDiv.className = 'record';
                recordDiv.innerHTML = `
                    <h3>Record ID: ${record.id}</h3>
                    <button onclick="deleteRecord(${record.id}, '${record.name}')">Delete Person</button>
                    <p>Name: ${record.name}</p>
                    <p>Job: ${record.job}</p>
                `;
                recordsElement.appendChild(recordDiv);
            }
        }
    } catch (error) {
        console.error("Error fetching records:", error);
    }
}

const deleteRecord = async (userId, userName) => {
    alert(`Delete User: ${userId} - ${userName}?`);
    try {
        const data = await callAPI('function=delete_record&parameters=' + userId); 
        console.log("Delete response:", data);
        fetchrecords();
    } catch (e) {
        console.error("Error deleting record ID:", e);
    }
}